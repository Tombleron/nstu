#include "server.c"
#include <windows.h>
#include <stdio.h>
#include <conio.h>

DWORD dwErrCode;
SERVICE_STATUS ss;
SERVICE_STATUS_HANDLE ssHandle;
#define MYServiceName "mySVC"
extern int Server();
extern int ServiceStart();
extern void ServiceStop();

void WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArv);
void WINAPI ServiceControl(DWORD dwControlCode);
void ReportStatus(DWORD dwCurrentState,	DWORD dwWin32ExitCode, DWORD dwWaitHint);
PROCESS_INFORMATION pid;

int SvcReportEvent(const char* text)
{

	DWORD res, Sz;
	HANDLE hFile;
	char buf[256];
	hFile = CreateFile("C:\\logfile.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (!hFile) return (-1);
	else
	{
		GetFileSize(hFile, &Sz);
		SetFilePointer(hFile, 0, NULL, FILE_END);
		sprintf(buf, "%s\r\n", text);
		WriteFile(hFile, buf, strlen(buf), &res, NULL);
		CloseHandle(hFile);
		return (int)res;
	}
}
// -----------------------------------------------------
// Функция main
// Точка входа процесса
// -----------------------------------------------------
void main(int agrc, char *argv[])
{
	char buffer[256];
	// Таблица точек входа
	SERVICE_TABLE_ENTRY DispatcherTable[] =
	{
		{
			// Имя сервиса
			MYServiceName,
			// Функция main сервиса
			(LPSERVICE_MAIN_FUNCTION)ServiceMain
		},
		{
			NULL,
			NULL
		}
	};
	SvcReportEvent("Sample service entry point");
	// Запуск диспетчера
	if (!StartServiceCtrlDispatcher(DispatcherTable))
	{
		sprintf(buffer,	"StartServiceCtrlDispatcher: Error %ld\n",GetLastError());
		SvcReportEvent(buffer);
		return;
	}
}
// -----------------------------------------------------
// Функция ServiceMain
// Точка входа сервиса 
// -----------------------------------------------------
void WINAPI ServiceMain(DWORD argc, LPSTR *argv)
{
	char buf[256];
	int res=0;
	// Регистрируем управляющую функцию сервиса
	ssHandle = RegisterServiceCtrlHandler(MYServiceName, ServiceControl);
	if (!ssHandle)
	{
		SvcReportEvent("Error RegisterServiceCtrlHandler");
	 return;
	}
	// Устанавливаем состояние сервиса
	// Сервис работает как отдельный процесс
	ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	// Код ошибки при инициализации и завершения сервиса не используется
	ss.dwServiceSpecificExitCode = 0;
	// Начинаем запуск сервиса.
	// Прежде всего устанавливаем состояние ожидания запуска сервиса
	ReportStatus(SERVICE_START_PENDING, NO_ERROR, 30000);
	SvcReportEvent("Service starting...");
	// Вызываем функцию, которая выполняет все необходимые инициализирующие действия
	res=ServiceStart();
	if (res < 0)
	{
		sprintf(buf, "Error init server %d", res);
		SvcReportEvent(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
		return;
	}
	// После завершения инициализации устанавливаем состояние работающего сервиса
	ReportStatus(SERVICE_RUNNING, NOERROR, 0);
	SvcReportEvent("Service started!");
	// основное тело службы
	if (Server() > 0)
	{
		SvcReportEvent("Server MF started!");
	}
	else
	{
		sprintf(buf, "Error starting server %d", res);
		SvcReportEvent(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
	}
	return;
}
// -----------------------------------------------------
// Функция ServiceControl
// Точка входа функции обработки команд
// -----------------------------------------------------
void WINAPI ServiceControl(DWORD dwControlCode)
{
	// Анализируем код команды и выполняем эту команду
	switch (dwControlCode)
	{
		// Команда остановки сервиса
	case SERVICE_CONTROL_STOP:
	{
		// Устанавливаем состояние ожидания остановки
		ss.dwCurrentState = SERVICE_STOP_PENDING;
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		SvcReportEvent("Service stopping...");
		// Выполняем остановку сервиса, вызывая функцию, которая выполняет все необходимые для этого действия
		ServiceStop();
		// Отмечаем состояние как остановленный сервис
		ReportStatus(SERVICE_STOPPED, NOERROR, 0);
		SvcReportEvent("Service stopped!");
		break;
	}
	// Определение текущего состояния сервиса
	case SERVICE_CONTROL_INTERROGATE:
	{
		// Возвращаем текущее состояние сервиса
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}
	// В ответ на другие команды просто возвращаем текущее состояние сервиса
	default:
	{
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}
	}
}
// -----------------------------------------------------
// Функция ReportStatus
// Посылка состояния сервиса системе управления сервисами
// -----------------------------------------------------
void ReportStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	// Счетчик шагов длительных операций
	static DWORD dwCheckPoint = 1;
	// Если сервис не находится в процессе запуска, его можно остановить
	if (dwCurrentState == SERVICE_START_PENDING)
		ss.dwControlsAccepted = 0;
	else
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	// Сохраняем состояние, переданное через параметры функции
	ss.dwCurrentState = dwCurrentState;
	ss.dwWin32ExitCode = dwWin32ExitCode;
	ss.dwWaitHint = dwWaitHint;
	// Если сервис не работает и не остановлен, увеличиваем значение счетчика шагов длительных операций
	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		ss.dwCheckPoint = 0;
	else
		ss.dwCheckPoint = dwCheckPoint++;
	// Вызываем функцию установки состояния
	SetServiceStatus(ssHandle, &ss);
}


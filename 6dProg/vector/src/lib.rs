use std::alloc::{self, Layout};
use std::marker::PhantomData;
use std::mem;
use std::ops::{Index, IndexMut, DerefMut, Deref};
use std::ptr::{self, NonNull};

pub struct MyVec<T> {
    ptr: NonNull<T>,
    cap: usize,
    len: usize,
    _marker: PhantomData<T>,
}

pub struct MyIter<T> {
    buf: NonNull<T>,
    cap: usize,
    start: *const T,
    end: *const T,
    _marker: PhantomData<T>,
}

impl<T> MyVec<T> {
    pub fn new() -> Self {
        MyVec {
            ptr: NonNull::dangling(),
            len: 0,
            cap: 0,
            _marker: PhantomData,
        }
    }

    pub fn with_capacity(cap: usize) -> Self {
        let layout = Layout::array::<T>(cap).unwrap();

        assert!(layout.size() <= isize::MAX as usize, "Allocation too large");

        let maybe_ptr = unsafe { alloc::alloc(layout) };

        let ptr = match NonNull::new(maybe_ptr as *mut T) {
            Some(p) => p,
            None => alloc::handle_alloc_error(layout),
        };

        Self {
            ptr,
            cap,
            len: 0,
            _marker: PhantomData,
        }
    }

    fn grow(&mut self) {
        let (new_cap, new_layout) = if self.cap == 0 {
            (1, Layout::array::<T>(1).unwrap())
        } else {
            let new_cap = 2 * self.cap;

            let new_layout = Layout::array::<T>(new_cap).unwrap();
            (new_cap, new_layout)
        };

        assert!(
            new_layout.size() <= isize::MAX as usize,
            "Allocation too large"
        );

        let new_ptr = if self.cap == 0 {
            unsafe { alloc::alloc(new_layout) }
        } else {
            let old_layout = Layout::array::<T>(self.cap).unwrap();
            let old_ptr = self.ptr.as_ptr() as *mut u8;
            unsafe { alloc::realloc(old_ptr, old_layout, new_layout.size()) }
        };

        self.ptr = match NonNull::new(new_ptr as *mut T) {
            Some(p) => p,
            None => alloc::handle_alloc_error(new_layout),
        };
        self.cap = new_cap;
    }

    pub fn push(&mut self, elem: T) {
        if self.len == self.cap {
            self.grow();
        }

        unsafe {
            ptr::write(self.ptr.as_ptr().add(self.len), elem);
        }

        self.len += 1;
    }

    pub fn pop(&mut self) -> Option<T> {
        if self.len == 0 {
            None
        } else {
            self.len -= 1;
            unsafe { Some(ptr::read(self.ptr.as_ptr().add(self.len))) }
        }
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn capacity(&self) -> usize {
        self.cap
    }

    pub fn into_iter(self) -> MyIter<T> {
        let ptr = self.ptr;
        let cap = self.cap;
        let len = self.len;

        mem::forget(self);

        unsafe {
            MyIter {
                buf: ptr,
                cap,
                start: ptr.as_ptr(),
                end: if cap == 0 {
                    ptr.as_ptr()
                } else {
                    ptr.as_ptr().add(len)
                },
                _marker: PhantomData,
            }
        }
    }
}

impl<T> Index<usize> for MyVec<T> {
    type Output = T;

    fn index(&self, index: usize) -> &Self::Output {
        assert!(
            index < self.len,
            "Index out of bounds: the len is {} but the index is {}",
            self.len,
            index
        );

        &self.deref()[index]
    }
}

impl<T> IndexMut<usize> for MyVec<T> {

    fn index_mut(&mut self, index: usize) -> &mut T {
        assert!(
            index < self.len,
            "Index out of bounds: the len is {} but the index is {}",
            self.len,
            index
        );

        &mut self.deref_mut()[index]
    }
}

impl<T> Deref for MyVec<T> {
    type Target = [T];
    fn deref(&self) -> &[T] {
        unsafe {
            std::slice::from_raw_parts(self.ptr.as_ptr(), self.len)
        }
    }
}

impl<T> DerefMut for MyVec<T> {
    fn deref_mut(&mut self) -> &mut [T] {
        unsafe {
            std::slice::from_raw_parts_mut(self.ptr.as_ptr(), self.len)
        }
    }
}

impl<T> Drop for MyVec<T> {
    fn drop(&mut self) {
        if self.cap != 0 {
            while let Some(_) = self.pop() {}
            let layout = Layout::array::<T>(self.cap).unwrap();
            unsafe {
                alloc::dealloc(self.ptr.as_ptr() as *mut u8, layout);
            }
        }
    }
}

impl<T> Iterator for MyIter<T> {
    type Item = T;
    fn next(&mut self) -> Option<T> {
        if self.start == self.end {
            None
        } else {
            unsafe {
                let result = ptr::read(self.start);
                self.start = self.start.offset(1);
                Some(result)
            }
        }
    }
}

impl<T> Drop for MyIter<T> {
    fn drop(&mut self) {
        if self.cap != 0 {
            // drop any remaining elements
            for _ in &mut *self {}
            let layout = Layout::array::<T>(self.cap).unwrap();
            unsafe {
                alloc::dealloc(self.buf.as_ptr() as *mut u8, layout);
            }
        }
    }
}

unsafe impl<T: Send> Send for MyVec<T> {}
unsafe impl<T: Sync> Sync for MyVec<T> {}

#[cfg(test)]
mod tests {
    use crate::MyVec;

    #[test]
    fn init() {

        let my_vec: MyVec<i64> = MyVec::new();

        assert_eq!(my_vec.len(), 0);
        assert_eq!(my_vec.capacity(), 0);

    }

    #[test]
    fn init_with_capacity() {
        let my_vec: MyVec<i64> = MyVec::with_capacity(10);

        assert_eq!(my_vec.capacity(), 10);
        assert_eq!(my_vec.len(), 0);
    }

    #[test]
    fn push() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        my_vec.push(2);
        my_vec.push(3);

        assert_eq!(my_vec.len(),  3);
        assert_eq!(my_vec.capacity(),  4);

    }

    #[test]
    fn pop() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        my_vec.push(2);
        my_vec.push(3);

        assert_eq!(my_vec.len(), 3);
        assert_eq!(my_vec.capacity(), 4);

        assert_eq!(my_vec.pop(), Some(3));
        assert_eq!(my_vec.len(), 2);
        assert_eq!(my_vec.capacity(), 4);
        
        assert_eq!(my_vec.pop(), Some(2));
        assert_eq!(my_vec.len(), 1);
        assert_eq!(my_vec.capacity(), 4);
    }

    #[test]
    fn iter() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        my_vec.push(2);
        my_vec.push(3);

        let mut iterator = my_vec.into_iter();

        assert_eq!(iterator.next(), Some(1));
        assert_eq!(iterator.next(), Some(2));
        assert_eq!(iterator.next(), Some(3));
        assert_eq!(iterator.next(), None);
    }

    #[test]
    fn index() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        my_vec.push(2);
        my_vec.push(3);
        
        assert_eq!(my_vec[0], 1);
        assert_eq!(my_vec[1], 2);
        assert_eq!(my_vec[2], 3);
    }

    #[test]
    #[should_panic]
    fn index_out_of_bound() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        
        my_vec[1];
    }

    #[test]
    fn index_mut() {
        let mut my_vec: MyVec<i64> = MyVec::new();

        my_vec.push(1);
        
        assert_eq!(my_vec[0], 1);

        my_vec[0] = 2;

        assert_eq!(my_vec[0], 2);
    }
}

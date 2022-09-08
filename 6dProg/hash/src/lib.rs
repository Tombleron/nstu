use std::collections::hash_map::DefaultHasher;
use std::collections::LinkedList;
use std::hash::{Hash, Hasher};

const SIZE: usize = 67;

struct MyHash<K, V>
where
    K: Hash,
{
    capacity: usize,
    buckets: Vec<Option<LinkedList<(K, V)>>>,
}

impl<K, V> Default for MyHash<K, V>
where
    K: Hash,
{
    fn default() -> Self {
        let mut arr = Vec::with_capacity(SIZE);
        for _ in 0..SIZE {
            arr.push(None);
        }
        Self {
            capacity: SIZE,
            buckets: arr,
        }
    }
}

impl<K, V> MyHash<K, V>
where
    K: Hash + PartialEq,
{
    fn new() -> Self {
        Default::default()
    }

    fn contains_key(&self, key: &K) -> bool {
        let mut hasher = DefaultHasher::default();
        key.hash(&mut hasher);
        let idx = hasher.finish() as usize % SIZE;

        if let Some(elem) = &self.buckets[idx] {
            elem.iter().any(|(k, _)| k == key)
        } else {
            false
        }
    }

    fn get(&self, k: &K) -> Option<&V> {
        

        
    }

    fn insert(&mut self, key: K, value: V) -> Option<V> {
        if self.contains(&key) {

        } else {
            let mut hasher = DefaultHasher::default();
            key.hash(&mut hasher);
            let idx = hasher.finish() as usize % SIZE;

            let mut new = LinkedList::new();
            new.push_front((key, value));

            self.buckets[idx] = Some(new);

            None
        }
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        let result = 2 + 2;
        assert_eq!(result, 4);
    }
}

use std::{collections::VecDeque, path::Iter, thread::current};

type Leaf<K, T> = Option<Box<BTreeNode<K, T>>>;

enum Find<K: Ord> {
    Count(u32),
    Result(K),
}

#[derive(Debug)]
struct BTreeNode<K: Ord, T> {
    value: T,
    key: K,
    left: Leaf<K, T>,
    right: Leaf<K, T>,
}

impl<K, T> BTreeNode<K, T>
where
    K: Ord,
{
    fn new(key: K, value: T) -> Self {
        Self {
            key,
            value,
            left: None,
            right: None,
        }
    }

    fn insert(&mut self, key: K, value: T) -> Option<T> {
        if self.key == key {
            return Some(std::mem::replace(&mut self.value, value));
        }

        let target_node = if key < self.key {
            &mut self.left
        } else {
            &mut self.right
        };

        match target_node {
            &mut Some(ref mut subnode) => subnode.insert(key, value),
            &mut None => {
                let new_node = BTreeNode::new(key, value);
                let boxed_node = Some(Box::new(new_node));
                *target_node = boxed_node;
                None
            }
        }
    }

    fn find(&self, index: u32, mut counter: u32) -> Find<&K> {
        if let Some(left) = self.left.as_ref() {
            let res = left.find(index, counter);

            match res {
                Find::Count(new_count) => counter = new_count,
                Find::Result(_key) => return res,
            }
        }

        if let Some(right) = self.right.as_ref() {
            let res = right.find(index, counter);

            match res {
                Find::Count(new_count) => counter = new_count,
                Find::Result(_key) => return res,
            }
        }

        counter += 1;
        if counter == index {
            Find::Result(&self.key)
        } else {
            Find::Count(counter)
        }
    }
}

#[derive(Debug)]
pub struct BTree<K: Ord, T> {
    head: Leaf<K, T>,
}

impl<K, T> BTree<K, T>
where
    K: Ord,
{
    pub fn new() -> Self {
        BTree {
            head: None,
            //    head: BTreeNode {
            //        value,
            //        left: None,
            //        right: None,
            //    },
        }
    }

    pub fn insert(&mut self, key: K, value: T) -> Option<T> {
        match self.head {
            Some(ref mut node) => node.insert(key, value),
            None => {
                self.head = Some(Box::new(BTreeNode::new(key, value)));
                None
            }
        }
    }

    pub fn lrt<F>(&self, f: F)
    where
        F: Fn(&T),
    {
        if self.head.is_none() {
            return;
        }

        let mut s1: VecDeque<&BTreeNode<K, T>> = VecDeque::new();
        let mut s2: VecDeque<&BTreeNode<K, T>> = VecDeque::new();

        let root = self.head.as_ref().unwrap();
        s1.push_back(root.as_ref());

        while !s1.is_empty() {
            let node = s1.pop_back().unwrap();
            s2.push_back(node);

            if let Some(left) = node.left.as_ref() {
                s1.push_back(left);
            }
            if let Some(right) = node.right.as_ref() {
                s1.push_back(right);
            }
        }

        while !s2.is_empty() {
            let node = s2.pop_back().unwrap();
            f(&node.value);
        }
    }

    pub fn find(&self, index: u32) -> Option<&K> {
        if let Some(elem) = &self.head {
            match elem.find(index, 0) {
                Find::Count(_) => None,
                Find::Result(res) => Some(res),
            }
        } else {
            None
        }
    }

    fn iter(&self) -> BTreeIterator<K, T> {
        let mut stack = VecDeque::new();
        let mut current = &self.head;

        while let Some(node) = current {
            current = &node.left;
            stack.push_back(node);
        }

        BTreeIterator { traversal: stack }
    }
}

struct BTreeIterator<'a, K: Ord, T> {
    traversal: VecDeque<&'a Box<BTreeNode<K, T>>>,
}

impl<'a, K, T> Iterator for BTreeIterator<'a, K, T>
where
    K: Ord,
{
    type Item = &'a Box<BTreeNode<K, T>>;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(node) = self.traversal.pop_back() {
            let mut current = &node.right;
            while let Some(elem) = current {
                current = &elem.left;
                self.traversal.push_back(&elem);
            }
            Some(node)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::BTree;

    #[test]
    fn build_tree_test() {
        let mut tree = BTree::<i32, String>::new();
        tree.insert(20, "a".to_string());
        tree.insert(10, "b".to_string());
        tree.insert(30, "c".to_string());
        tree.insert(5, "d".to_string());
    }

    #[test]
    fn lrt_test() {
        let mut tree = BTree::<i32, String>::new();
        tree.insert(20, "a".to_string());
        tree.insert(10, "b".to_string());
        tree.insert(30, "c".to_string());
        tree.insert(5, "d".to_string());
        tree.insert(15, "e".to_string());
        tree.insert(25, "f".to_string());
        tree.insert(40, "g".to_string());
        tree.insert(12, "h".to_string());
        tree.insert(17, "i".to_string());
        tree.insert(35, "j".to_string());

        tree.lrt(|x| print!(" {x} "));
    }

    #[test]
    fn find() {
        let mut tree = BTree::<i32, String>::new();
        tree.insert(20, "a".to_string());
        tree.insert(10, "b".to_string());
        tree.insert(30, "c".to_string());
        tree.insert(5, "d".to_string());
        tree.insert(15, "e".to_string());
        tree.insert(25, "f".to_string());
        tree.insert(40, "g".to_string());
        tree.insert(12, "h".to_string());
        tree.insert(17, "i".to_string());
        tree.insert(35, "j".to_string());

        assert_eq!(tree.find(5), Some(&10));
        assert_eq!(tree.find(1000), None);
    }

    #[test]
    fn iterator() {
        let mut tree = BTree::<i32, String>::new();
        tree.insert(1, "a".to_string());
        tree.insert(2, "b".to_string());
        tree.insert(3, "c".to_string());

        let mut i = tree.iter();

        assert!(i.next().is_some());
        assert!(i.next().is_some());
        assert!(i.next().is_some());
        assert!(i.next().is_none());
    }
}

#include <malloc.h>

template<class T> class Collection {
protected:
	unsigned long count;
public:
	Collection(): count(0) {}

	virtual void clear() = 0;
	virtual void add(const T &val) = 0;
	virtual const bool remove(const T &val) = 0;

	const unsigned long size() const {return count;}
};

template<class T> class Node {
public:
	T val;

	Node(const T &v): val(v) {}
	virtual ~Node() {}
};

template<class T> class ListNode: public Node<T> {
public:
	ListNode<T> *next, *prev;

	ListNode(const T &v): Node<T>(v), next(0), prev(0) {}
	virtual ~ListNode() {
		if(next) next->prev = prev;
		if(prev) prev->next = next;			
	}
};

template<class T> class LinkedList: public Collection<T> {
protected:
	ListNode<T> *head, *tail;

public:
	class Iterator {
		friend class LinkedList<T>;
	protected:
		ListNode<T> *n;
		Iterator(ListNode<T> *start): n(start) {}
	public:
		Iterator(const Iterator &other): n(other.n) {}

		virtual T &val() {return n->val;}
		virtual void next() {if(n) n = n->next;}
		virtual void prev() {if(n) n = n->prev;}
		virtual const bool has_val() {return (n ? true : false); }
	};

	LinkedList(): Collection<T>(), head(0), tail(0) {};
	LinkedList(const LinkedList<T> &other): Collection<T>(), head(0), tail(0) {
		for(Iterator i = other.start(); i.has_val(); i.next())
			add(i.val());
	}
	virtual ~LinkedList() {clear();}

	LinkedList<T> &operator=(const LinkedList<T> &other) {
		clear();
		for(Iterator i = other.start(); i.has_val(); i.next())
			add(i.val());
		return *this;
	}

	virtual void clear() {
		ListNode<T> *n;
		while(head) {
			n = head;
			head = head->next;
			delete n;
		}
		tail = 0;
		Collection<T>::count = 0;
	}

	virtual Iterator start() const {return Iterator(head);}

	virtual void add_front(T &val) {
		ListNode<T> *n = new ListNode<T>(val);
		n->next = head;
		if(head) head->prev = n;
		head = n;
		if(!tail) tail = n;
		Collection<T>::count++;
	}

	virtual void add(const T &val) {
		ListNode<T> *n = new ListNode<T>(val);
		n->prev = tail;
		if(tail) tail->next = n;
		tail = n;
		if(!head) head = n;
		Collection<T>::count++;
	}

	virtual const bool remove(const T &val) {
		ListNode<T> *n = head;
		while(n) {
			if(n->val == val) {
				if(n == head) head = head->next;
				if(n == tail) tail = tail->prev;
				
				delete n;
				Collection<T>::count--;
				return true;
			} else
				n = n->next;
		}

		return false;
	}

	virtual const bool contains(T &val) const {
		ListNode<T> *n = head;
		while(n) {
			if(n->val == val) {
				return true;
			} else
				n = n->next;
		}

		return false;
	}

	// queue/stack functions
	// stack - use push/pop
	// queue - use push_back/pop
	virtual void push(T val) {
		add_front(val);
	}

	virtual void push_back(T &val) {
		add(val);
	}

	virtual const bool pop(T &val) {
		if(!head) return false;

		ListNode<T> *n = head;
		if(head) {
			head = head->next;
			if(n == tail) tail = 0;
			val = n->val;
			delete n;
			Collection<T>::count--;
			return true;
		} else
			return false;
	}
};

template<class T> class DynamicArray: public Collection<T> {
protected:
	T *ar;

	unsigned long initial, limit, increment;

public:
	class Iterator {
		friend class DynamicArray<T>;
	protected:
		T *ar;
		unsigned long count;
		unsigned long pos;
		Iterator(T *a, const int c, unsigned long p): ar(a), count(c), pos(p) {}
	public:
		Iterator(const Iterator &other): ar(other.ar), count(other.count), pos(other.pos) {}

		virtual T &val() {return ar[pos];}
		virtual void next() {pos++;}
		virtual void prev() {pos--;}
		virtual const bool has_val() {return pos < count; }
	};

	DynamicArray(unsigned long init = 0, unsigned long inc = 1): Collection<T>(), ar(0), initial(init), limit(init), increment(inc) {
		if(limit) ar = (T *)malloc(limit * sizeof(T));
	}
	virtual ~DynamicArray() {if(ar) free(ar);}

	virtual void clear() {
		Collection<T>::count = 0;
		limit = initial;
		if(limit) ar = (T *)realloc(ar, limit * sizeof(T));
		else {
			free(ar);
			ar = 0;
		}
	}

	virtual Iterator start() const {return Iterator(ar, Collection<T>::count, 0);}

	virtual void add(const T &val) {
		if(Collection<T>::count == limit) {
			limit += increment;
			ar = (T *)realloc(ar, limit * sizeof(T));
			ar[Collection<T>::count++] = val;
		} else
			ar[Collection<T>::count++] = val;
	}

	virtual void add_all(DynamicArray<T> &other) {
		for(Iterator i = other.start(); i.has_val(); i.next()) {
			add(i.val());
		}
	}

	virtual const bool remove(const T &val) {
		for(unsigned long i = 0; i < Collection<T>::count; i++) {
			if(ar[i] == val) {
				memmove(ar + i, ar + i + 1, (Collection<T>::count - i) * sizeof(T));
				Collection<T>::count--;
				return true;
			}
		}
		return false;
	}

	virtual const bool remove(const unsigned long index) {
		if(index >= Collection<T>::count) return false;

		memmove(ar + index, ar + index + 1, (Collection<T>::count - index) * sizeof(T));
		Collection<T>::count--;
		return true;
	}

	virtual const bool insert(const T &val, const unsigned long index) {
		if(index > Collection<T>::count) return false;

		if(Collection<T>::count == limit) {
			limit += increment;
			ar = (T *)realloc(ar, limit * sizeof(T));
		}

		if(index < Collection<T>::count)
			memmove(ar + index + 1, ar + index, (Collection<T>::count - index) * sizeof(T));
		
		ar[index] = val;
		Collection<T>::count++;
		return true;
	}

	virtual T &operator[](const int index) {
		return ar[index];
	}

	const bool index_of(const T &val, unsigned long &index) const {
		for(int i = 0; i < Collection<T>::count; i++) {
			if(ar[index] == val) {
				index = i;
				return true;
			}
		}
		return false;
	}

	const int index_of(const T &val) const {
		for(int i = 0; i < Collection<T>::count; i++) {
			if(ar[i] == val) {
				return i;
			}
		}
		return -1;
	}

	// stack functions
	virtual const bool pop(T &val) {
		if(Collection<T>::count) {
			val = ar[Collection<T>::count -1];
			remove(Collection<T>::count -1);
			return true;
		}
		return false;
	}

	virtual void push(const T &val) {
		add(val);
	}
};

template<class T> class SortedDynamicArray: public DynamicArray<T> {
public:
	SortedDynamicArray(unsigned long init = 0, unsigned long inc = 1): DynamicArray<T>(init, inc) {}
	virtual ~SortedDynamicArray() {}

	const bool get_index(const T &val, unsigned long &index) {
		unsigned long low  = 0;
		unsigned long high = Collection<T>::count-1;

		while( high < Collection<T>::count && low <= high )
		{  
			unsigned long i = ( low+high )/2;
			if ( DynamicArray<T>::ar[i] == val )
			{	index = i;
				return true;
			} else 
				if (DynamicArray<T>::ar[i] < val)
					low = i+1;
				else
					high = i-1;
		}

		index = low;
		return false;
	}

	virtual void add(const T &val) {
		unsigned long index;
		get_index(val, index);
		insert(val, index);
	}
};

template<class T> class TreeNode: public Node<T> {
public:
	TreeNode<T> *parent, *left, *right;

	TreeNode(const T &v, TreeNode<T> *par): Node<T>(v), parent(par), left(0), right(0) {}
	virtual ~TreeNode() {
		if(parent) {
			if(parent->left == this) parent->left = 0;
			if(parent->right == this)parent->right = 0;
		}
	}
};

template<class T, class N = TreeNode<T> > class BinaryTree: public Collection<T> {
protected:

	N *root;

	virtual void delete_node(N *n) {
		if(n->left && n->right) {
			N *minmax = n->left;
			while(minmax->right) minmax = minmax->right;
			n->val = minmax->val;
			delete_node(minmax);
			return;
		} else if(n->right) {
			if(n->parent) {
				if(n->parent->left == n) n->parent->left = n->right;
				else n->parent->right = n->right;
			} else
				root = n->right;
			n->right->parent = n->parent;
		} else if(n->left) {
			if(n->parent) {
				if(n->parent->left == n)	n->parent->left = n->left;
				else n->parent->right = n->left;
			} else
				root = n->left;
			n->left->parent = n->parent;
		} else {
			if(n == root) root = 0;
		}
		delete n;
		Collection<T>::count--;
	}

	virtual void insert_node(N *n) {
		N *current = root, *parent = 0;
		while(current) {
			parent = current;
			if(n->val < current->val)
				current = current->left;
			else
				current = current->right;
		}

		if(parent) {
			if(n->val < parent->val) {
				parent->left = n;
			} else {
				parent->right = n;
			}
		} else
			root = n;

		n->parent = parent;
		Collection<T>::count++;
	}

public:
	class Iterator {
		friend class BinaryTree<T, N>;
	protected:

		class EvalNode {
		public:
			bool evaluate;
			N *node;

			EvalNode(): evaluate(false), node(0) {}
			EvalNode(const bool eval, N *n): evaluate(eval), node(n) {}
			const bool operator==(const EvalNode &other) const {return node == other.node;}
			EvalNode &operator=(const EvalNode &other) {evaluate = other.evaluate; node = other.node; return *this;}

		};

		N *n;
		LinkedList<EvalNode> stack;

		
		Iterator(N *start): n(0) {
			if(start) {
				stack.push(EvalNode(true, start));
				next();
			}
		}

	public:
		Iterator(const Iterator &other):n(other.n), stack(other.stack)  {}
		virtual ~Iterator() {}

		virtual T &val() {return n->val;}
		virtual void next() {
			EvalNode en;
			bool popped = false;
			while((popped = stack.pop(en)) && en.evaluate) {
				if(en.node->right) stack.push(EvalNode(true, en.node->right));
				stack.push(EvalNode(false, en.node));
				if(en.node->left) stack.push(EvalNode(true, en.node->left));
			}
			
			n = (popped ? en.node : 0);
		}
		virtual const bool has_val() {return (n ? true : false);}
	};

	BinaryTree(): Collection<T>(), root(0) {};
	BinaryTree(BinaryTree<T> &other): Collection<T>(), root(0) {
		for(Iterator i = other.start(); i.has_val(); i.next())
			add(i.val());
	}
	virtual ~BinaryTree() {clear();}

	BinaryTree &operator=(BinaryTree<T> &other) {
		clear();
		for(Iterator i = other.start(); i.has_val(); i.next())
			add(i.val());
		return *this;
	}

	virtual void clear() {
		N *current = root, *parent = 0;
		while(current) {
			if(current->left) current = current->left;
			else if(current->right) current = current->right;
			else {
				parent = current->parent;
				delete current;
				current = parent;
			}
		}

		root = 0;
		Collection<T>::count = 0;
	}

	void add(const T &val) {
		N *n = new N(val, 0);
		insert_node(n);
	}

	const bool remove(const T &val) {
		N *current = root;
		while(current) {
			if(current->val == val)
				break;
			else if(val < current->val)
				current = current->left;
			else 
				current = current->right;
		}

		if(current) {
			delete_node(current);
			return true;
		}

		return false;
	}

	const bool contains(const T &val) const {
		N *current = root;
		while(current) {
			if(current->val == val)
				break;
			else if(val < current->val)
				current = current->left;
			else 
				current = current->right;
		}

		return current != 0;
	}

	Iterator start() const {return Iterator(root);}
};

#define RED		1
#define BLACK	0

// thanks to wikipedia (http://en.wikipedia.org/wiki/Red_black_tree)
template<class T> class ColouredTreeNode: public Node<T> {
public:
	ColouredTreeNode<T> *parent, *left, *right;
	char color;

	ColouredTreeNode(const T &v, ColouredTreeNode<T> *par): Node<T>(v), parent(par), left(0), right(0), color(BLACK) {}
	virtual ~ColouredTreeNode() {
		if(parent) {
			if(parent->left == this) parent->left = 0;
			if(parent->right == this)parent->right = 0;
		}
	}
};

template<class T, class N = ColouredTreeNode<T> > class RedBlackTree: public BinaryTree<T, N> {
protected:
	N *grandparent(N *n) {
		if (n && n->parent)
			return n->parent->parent;
		else
			return NULL;
	}
	N *uncle(N *n) {
		if(grandparent(n)) {
			if (n->parent == grandparent(n)->left)
				return grandparent(n)->right;
			else
				return grandparent(n)->left;
		} else
			return NULL;
	}
	N *sibling(N *n) {
		if(n->parent) {
			if (n == n->parent->left)
				return n->parent->right;
			else
				return n->parent->left;
		} else
			return NULL;
	}
	bool is_leaf(N *n) {
		return n == 0;
	}

	void replace_node(N *o, N *n) {
		n->parent = o->parent;
		if(n->parent) {
			if(n->parent->left == o) n->parent->left = n;
			else if(n->parent->right == o) n->parent->right = n;
		} else
			BinaryTree<T, N>::root = n;
	}

	void rotate_left(N *n) {
		N *p = n->right;
		N *q = n;

		q->right = p->left;
		if(q->right) q->right->parent = q;
		p->left = q;
		p->parent = q->parent;
		q->parent = p;
		if(p->parent) {
			if(p->parent->left == q) p->parent->left = p;
			else if(p->parent->right == q) p->parent->right = p;
		} else
			BinaryTree<T, N>::root = p;
	}
	void rotate_right(N *n) {
		N *p = n->left;
		N *q = n;

		q->left = p->right;
		if(q->left) q->left->parent = q;
		p->right = q;
		p->parent = q->parent;
		q->parent = p;
		if(p->parent) {
			if(p->parent->left == q) p->parent->left = p;
			else if(p->parent->right == q) p->parent->right = p;
		} else
			BinaryTree<T, N>::root = p;
	}

	void insert_case1(N *n) {
		if (n->parent == NULL)
			n->color = BLACK;
		else
			insert_case2(n);
	}
	void insert_case2(N *n) {
		if (n->parent->color == BLACK)
			return; /* Tree is still valid */
		else
			insert_case3(n);
	}
	void insert_case3(N *n) {
		if (uncle(n) != NULL && uncle(n)->color == RED) {
			n->parent->color = BLACK;
			uncle(n)->color = BLACK;
			grandparent(n)->color = RED;
			insert_case1(grandparent(n));
		} else
			insert_case4(n);
	}
	void insert_case4(N *n) {
		if (n == n->parent->right && n->parent == grandparent(n)->left) {
			rotate_left(n->parent);
			n = n->left;
		} else if (n == n->parent->left && n->parent == grandparent(n)->right) {
			rotate_right(n->parent);
			n = n->right;
		}
		insert_case5(n);
	}
	void insert_case5(N *n) {
		n->parent->color = BLACK;
		grandparent(n)->color = RED;
		if (n == n->parent->left && n->parent == grandparent(n)->left) {
			rotate_right(grandparent(n));
		} else {
			/* Here, n == n->parent->right && n->parent == grandparent(n)->right */
			rotate_left(grandparent(n));
		}
	}

	void delete_case0(N *n) {
		/* Precondition: n has at most one non-null child */
		N *child = is_leaf(n->right) ? n->left : n->right;
		if(child) replace_node(n, child);
		if (n->color == BLACK) {
			if(child) {
				if (child->color == RED)
					child->color = BLACK;
				else
					delete_case1(child);
			} else
				delete_case1(n);
		}
		if(BinaryTree<T, N>::root == n) BinaryTree<T, N>::root = 0;
		delete n;
		Collection<T>::count--;
	}
	void delete_case1(N *n) {
		if (n->parent == NULL)
			return;
		else
			delete_case2(n);
	}
	void delete_case2(N *n) {
		if (sibling(n) && sibling(n)->color == RED) {
			n->parent->color = RED;
			sibling(n)->color = BLACK;
			if (n == n->parent->left)
				rotate_left(n->parent);
			else
				rotate_right(n->parent);
		}
		delete_case3(n);
	}
	void delete_case3(N *n) {
		if (n->parent->color == BLACK &&
			sibling(n) &&
			sibling(n)->color == BLACK &&
			(sibling(n)->left == 0 || sibling(n)->left->color == BLACK) &&
			(sibling(n)->right == 0 || sibling(n)->right->color == BLACK))
		{
			sibling(n)->color = RED;
			delete_case1(n->parent);
		} else
			delete_case4(n);
	}
	void delete_case4(N *n) {
		if (n->parent->color == RED &&
			sibling(n) &&
			sibling(n)->color == BLACK &&
			(sibling(n)->left == 0 || sibling(n)->left->color == BLACK) &&
			(sibling(n)->right == 0 || sibling(n)->right->color == BLACK))
		{
			sibling(n)->color = RED;
			n->parent->color = BLACK;
		} else
			delete_case5(n);
	}
	void delete_case5(N *n) {
		if (n == n->parent->left &&
			sibling(n) && 
			sibling(n)->color == BLACK &&
			sibling(n)->left && 
			sibling(n)->left->color == RED &&
			(sibling(n)->right == 0 || sibling(n)->right->color == BLACK))
		{
			sibling(n)->color = RED;
			sibling(n)->left->color = BLACK;
			rotate_right(sibling(n));
		} else if (n == n->parent->right &&
			sibling(n) && 
			sibling(n)->color == BLACK &&
			sibling(n)->right && 
			sibling(n)->right->color == RED &&
			(sibling(n)->left == 0 || sibling(n)->left->color == BLACK))
		{
			sibling(n)->color = RED;
			sibling(n)->right->color = BLACK;
			rotate_left(sibling(n));
		}
		delete_case6(n);
	}
	void delete_case6(N *n) {
		sibling(n)->color = n->parent->color;
		n->parent->color = BLACK;
		if (n == n->parent->left) {
			/* Here, sibling(n)->right->color == RED */
			sibling(n)->right->color = BLACK;
			rotate_left(n->parent);
		} else {
			/* Here, sibling(n)->left->color == RED */
			sibling(n)->left->color = BLACK;
			rotate_right(n->parent);
		}
	}

	N *get_predecessor(N *n) {
		N *minmax = n->left;
		while(minmax->right) minmax = minmax->right;
		return minmax;
	}

	virtual void insert_node(N *n) {
		BinaryTree<T, N>::insert_node(n);
		n->color = RED;

		insert_case1(n);
	}

	virtual void delete_node(N *n) {
		if(n->left && n->right) {
			N *predecessor = get_predecessor(n);
			n->val = predecessor->val;
			delete_case0(predecessor);
		} else
			delete_case0(n);
	}

public:
	RedBlackTree(): BinaryTree< T, N >() {}
	virtual ~RedBlackTree() {}
};

template<class A, class B> class Pair {
public:
	A first;
	B second;

	Pair(const A &f): first(f) {}
	Pair(const A &f, const B &s): first(f), second(s) {}
	Pair(const Pair<A,B> &other): first(other.first), second(other.second) {}
	virtual ~Pair() {}

	const bool operator<(const Pair<A,B> &other) const {return first < other.first;}
	const bool operator==(const Pair<A,B> &other) const {return first == other.first;}
	Pair<A,B> &operator=(const Pair<A,B> &other) {first = other.first; second = other.second; return *this;}
};

//template<class A, class B, class N = TreeNode<Pair<A, B> > > class Map: public BinaryTree< Pair< A, B >, N > {
template<class A, class B, class N = ColouredTreeNode<Pair<A, B> > > class Map: public RedBlackTree< Pair< A, B >, N > {
protected:
	
	N *find(A &key) const {
		N *n = RedBlackTree< Pair< A, B >, N >::root;
		while(n) {
			if(n->val.first == key)
				return n;
			else if(key < n->val.first)
				n = n->left;
			else
				n = n->right;

		}
		return 0;
	}
public:
	//Map(): BinaryTree< Pair<A,B>, N >() {}
	Map(): RedBlackTree< Pair<A,B>, N >() {}
	virtual ~Map() {}

	void put(A &key, B &value) {
		add(Pair<A,B>(key, value));
	}

	const bool get(A &key, B &val) const {
		const N *n = find(key);
		if(n) {
			val = n->val.second;
			return true;
		} else
			return false;
	}

	B &operator[](A &key) {
		N *n = find(key);
		if(n)
			return n->val.second;
		else {
			Pair< A, B > p(key);
			N *n = new N(p, 0);
			insert_node(n);
			return n->val.second;
		}
	}

	virtual const bool exists(A &key) const {
		const N *n = find(key);
		if(n) {
			return true;
		} else
			return false;
	}

	virtual const bool remove(A &key) {
		N *n = find(key);
		if(n) {
			delete_node(n);
			return true;
		} else
			return false;
	}
};

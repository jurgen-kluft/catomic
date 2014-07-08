#ifndef __XMULTICORE_DLIST_H__
#define __XMULTICORE_DLIST_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

/**
* @file xatomic\dlist.h
* Generic circular doubly linked list
*
* Head:            +------+------+
*       +--------->| next | prev |----------------------------------------+
*       |          +------+------+                                        |
*       |              |      ^                                           |
*       |              |      |                                           |
*       |   +----------+      +----------------------------------------+  |
*       |   |                                                          |  |
*       |   |  +-----------+       +-----------+        +-----------+  |  |
* List: |   |  | item1     |       | item2     |        | itemN     |  |  |
*       |   |  |           |       |           |        |           |  |  |
*       |   |  +-----------+       +-----------+        +-----------+  |  |
*       |   +->|   next    |------>|   next    | ... -->|   next    |--+  |
*       |      +-----------+       +-----------+        +-----------+     |
*       +------|   prev    |<------|   prev    | ... <--|   prev    |<----+
*              +-----------+       +-----------+        +-----------+
*              |           |       |           |        |           |
*              +-----------+       +-----------+        +-----------+
*
* Why use this list instead of the STL list ?
* - Zero allocations.
*   Does not need an allocator.
* - List head is a node, node is a head.
*   Most list operations can be done without having to know the actual head.
*/

namespace xcore
{
	namespace dlist
	{
		class head;
		class node;

		/**
		* List node. Nodes can be added to the list or chained.
		* @see head
		*
		* Chain is basically singly linked / null terminated list.
		* Users are expected to do all the hard-work. ie initialize, set,
		* clear, null-terminate, etc. However some of the list methods are 
		* chain friendly, chop() for example, for efficiency.
		* Node can be either in a chain or in a list, not both.
		* @see chop()
		*/
		class node 
		{
			friend class head;
		protected:
			node*		_next;
			node*		_prev;

			/**
			* Insert a new entry between two known consecutive entries. 
			*
			* This is only for internal list manipulation where we know
			* the prev/next entries already!
			*/
			void __add(dlist::node* _node, dlist::node* _prev, dlist::node* _next)
			{
				_next->_prev = _node;
				_node->_next = _next;
				_node->_prev = _prev;
				_prev->_next = _node;
			}

			/**
			* Delete a list entry by making the prev/next entries
			* point to each other.
			*
			* This is only for internal list manipulation where we know
			* the prev/next entries already!
			*/
			void __del(dlist::node *prev, dlist::node *next)
			{
				next->_prev = prev;
				prev->_next = next;
			}

		public:
			node*			next() const										{ return _next; }
			node*			prev() const										{ return _prev; }

			/**
			* Chain a node with this node.
			* Used for chaining (singly linked list).
			* To null-terminate the chain use n.chain(0)
			* @n node to chain 
			*/
			void			chain(node *n)										{ _next = n; }

			/**
			* Add a node after this node.
			* Used for insertions to the list.
			* @node: node to add
			*/
			void			add(node *node)										{ __add(node, this, _next); }

			/**
			* Insert a node after a give node already on the list.
			* @after: Insertion point
			* @node: Node to insert
			*
			* Insert a new entry before the specified head.
			* This is useful for implementing queues.
			*/
			void			insert(node *after, node *node)						{ after->add(node); }

			/**
			* Prepends a new entry
			* @node new entry to be added
			*/
			void			prepend(node *node)									{ add(node); }

			/**
			* Deletes entry from list.
			* @entry: the element to delete from the list.
			*/
			void			del(node *node)										{ __del(node->_prev, node->_next); }
		};

		template <class T = node> class iterator;

		class head : public node 
		{
			friend class iterator<>;

		public: 
			void				reset()											{ _next = this; _prev = this; }

								head()											{ reset(); }
								~head()											{ }

			/**
			* Append a new entry
			* @new: new entry to be added
			* @head: list head to add it before
			*
			* Insert a new entry before the specified head.
			* This is useful for implementing queues.
			*/
			void				append(node *node)								{ __add(node, _prev, this); }

			/**
			* Tests whether a list is empty
			* @head: the list to test.
			*/
			bool				empty() const									{ return _next == this; }

			/**
			* Chop off first part of the list up to this node.
			* Properly terminates node chain (ie head -> chopping point).
			* @param node the node chopping point
			*/
			void				chop(node *node)								{ __del(this, node->_next); node->_next = 0; node->_prev = 0; }

			/**
			* Push new node to the top of the list.
			* @param node node to push
			*/
			void				push(node *n)									{ prepend(n); }

			/**
			* Pop a node from the top of the list.
			* @return top node, 0 if empty
			*/
			node*				pop()
			{
				if (empty())
					return 0;
				node *n = next();
				del(n);
				return n;
			}

			/** 
			* Splice another list with this one.
			* @param head other list to splice
			*/
			void				splice(head *head)
			{
				if (head->empty())
					return;

				/**
				 * See the picture at the top to understand how this works.
				 * Basically we disconnect first and last node for the new
				 * list and reconnect them to 'this' list.
				 * Requires four pointer manipulations, just like inserting 
				 * a node into the list.
                 */
				node *first  = head->_next;
				node *last   = head->_prev;

				first->_prev = _prev;
				last->_next  = this;
				_prev->_next = first;
				this->_prev  = last;
			}

			/**
			* Appends the given list at the end of this list
			* and remove the nodes from given list.
			* @head: list head that needs to be appended to this list. 
			*/
			void				join(head *head)
			{
				splice(head);
				head->reset();
			}

			/**
			* Get total number of nodes in this list
			*/
			u32					count() const;

			const node*			begin() const									{ return this; }
			const node*			end()   const									{ return this; }
		};

		/** 
		* Simple list iterator.
		* Iterates over the list in both directions.
		* Node returned by next() can be removed in the same context.
		* In other words iterator is safe, as long as direction does
		* not change.
		*/
		template <class T>
		class iterator 
		{
		public:
			/**
			* Get next node (ie iterate forward).
			* @return pointer to the next node or 0.
			*/
			T* next()
			{
				node *n = _node;
				if (n == _head)
					return 0;
				_node = n->next();
				return (T*) n;
			}

			/**
			* Get prev node (ie iterate backward).
			* @return pointer to the next node or 0.
			*/
			T* prev()
			{
				node *n = _node;
				if (n == _head)
					return 0;
				_node = n->prev();
				return (T *) n;
			}

			/**
			* Reset the iterator to iterate over a new list.
			* @param head pointer to the list to iterate over
			* @param forward true - iterate forward (default), backward otherwise.
			*/
			void iterate(const head *head, bool forward = true)
			{
				_head = head;
				if (forward)
					_node = head->next();
				else
					_node = head->prev();
			}

			/**
			* Allocate and initialize iterator.
			* @param head pointer to the list to iterate over
			* @param forward true - iterate forward (default), backward otherwise.
			*/
			iterator(const head *head, bool forward = true)
			{ 
				iterate(head, forward);
			}
			~iterator() {};

		private:
			const head *_head;
			node       *_node;
		};

		/** 
		* STL style list iterator.
		* Not safe with respect to deletions.
		*/
		template <class T = node>
		class stl_iterator 
		{
		public:
			void		operator=(const node *n)				{ _node = static_cast <T*> (n->next()); }
			bool		operator!=(const node *n) const			{ return _node != n; }
			bool		operator==(const node *n) const			{ return _node == n; }
			void		operator++(int)							{ _node = static_cast <T*> (_node->next()); }
			const T&	operator++(void)						{ _node = static_cast <T*> (_node->next()); return *this; }
			T*			operator->() const						{ return _node; }

		private:
			T*			_node;
		};

		/**
		 * This can only be implemented after iterator is defined
		 */
		inline u32 head::count() const
		{
			u32 c = 0;
			stl_iterator<> iter;
			for (iter = begin(); iter != end(); iter++) c++;
			return c;
		}

	} // namespace dlist
} // namespace xcore

#endif ///< __XMULTICORE_DLIST_H__

#ifndef _LLIST_H_
#define _LLIST_H_

struct Node {
  int ID;
  Node *next;
};

void AddNode(Node **list, int ID);

int RemoveNode(Node **list, int ID);

#endif

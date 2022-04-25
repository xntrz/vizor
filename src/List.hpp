#pragma once


struct ListNode_t
{
    ListNode_t* Next;
    ListNode_t* Prev;
};


struct List_t
{
    ListNode_t* Head;
    ListNode_t* Tail;
};


void ListInit(List_t* List);
void ListInsert(List_t* List, ListNode_t* Node);
void ListRemove(List_t* List, ListNode_t* Node);
void ListReverse(List_t* List);
bool ListEmpty(List_t* List);
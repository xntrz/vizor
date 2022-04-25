#include "List.hpp"


void ListInit(List_t* List)
{
    List->Head = nullptr;
    List->Tail = nullptr;
};


void ListInsert(List_t* List, ListNode_t* Node)
{
    Node->Next = nullptr;
    Node->Prev = nullptr;

    if (List->Head == nullptr)
    {
        List->Head = Node;
        List->Tail = Node;
    }
    else
    {
        Node->Prev = List->Tail;
        List->Tail->Next = Node;
        List->Tail = Node;
    };    
};


void ListRemove(List_t* List, ListNode_t* Node)
{
    if (Node->Prev)        
        Node->Prev->Next = Node->Next;
    
    if (Node->Next)
        Node->Next->Prev = Node->Prev;

    if (List->Tail == Node)
        List->Tail = Node->Prev;

    if (List->Head == Node)
        List->Head = Node->Next;
};


void ListReverse(List_t* List)
{
    ;
};


bool ListEmpty(List_t* List)
{
    return ((List->Head == nullptr) &&
            (List->Tail == nullptr));
};
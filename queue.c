#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *const head = malloc(sizeof(*head));
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    for (struct list_head *node = l->next; node != l;) {
        struct list_head *node_deleting = node;
        element_t *element_deleting =
            list_entry(node_deleting, element_t, list);
        node = node->next;
        // free(node_deleting);
        q_release_element(element_deleting);
    }
    free(l);
}

static element_t *q_new_element(char *s)
{
    element_t *const element_inserting = malloc(sizeof(*element_inserting));
    if (!element_inserting)
        return NULL;
    const size_t string_size = strlen(s) + 1;
    element_inserting->value = malloc(string_size);
    if (!element_inserting->value)
        goto free_element;
    strncpy(element_inserting->value, s, string_size);
    return element_inserting;
free_element:
    free(element_inserting);
    return NULL;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *const element_inserting = q_new_element(s);
    if (!element_inserting)
        return false;
    list_add(&element_inserting->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *const element_inserting = q_new_element(s);
    if (!element_inserting)
        return false;
    list_add_tail(&element_inserting->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node_removing = head->next;
    list_del_init(node_removing);
    if (sp) {
        char *const value = list_entry(node_removing, element_t, list)->value;
        const size_t value_length_p1 = strlen(value) + 1;
        if (bufsize < value_length_p1) {
            strncpy(sp, value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        } else {
            strncpy(sp, value, value_length_p1);
        }
    }
    return list_entry(node_removing, element_t, list);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node_removing = head->prev;
    list_del_init(node_removing);
    if (sp) {
        char *const value = list_entry(node_removing, element_t, list)->value;
        const size_t value_length_p1 = strlen(value) + 1;
        if (bufsize < value_length_p1) {
            strncpy(sp, value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        } else {
            strncpy(sp, value, value_length_p1);
        }
    }
    return list_entry(node_removing, element_t, list);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    struct list_head *node;
    if (!head)
        return 0;
    int result = 0;
    list_for_each (node, head)
        ++result;
    return result;
}

static bool q_delete(struct list_head *node)
{
    if (!node)
        return false;
    list_del(node);
    q_release_element(list_entry(node, element_t, list));
    return true;
}

static struct list_head *q_get_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || head->next == head)
        return NULL;
    struct list_head *node_runner, *node_walker;
    for (node_runner = node_walker = head->next;
         (node_runner = node_runner->next) != head &&
         (node_runner = node_runner->next) != head;
         node_walker = node_walker->next)
        ;
    return node_walker;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    return q_delete(q_get_mid(head));
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || head->prev == head->next)
        return true;
    bool is_duplicate_prev = false;
    for (struct list_head *node_distinct = head; node_distinct->next != head;) {
        const bool is_duplicate =
            node_distinct->next->next != head &&
            !strcmp(
                list_entry(node_distinct->next, element_t, list)->value,
                list_entry(node_distinct->next->next, element_t, list)->value);
        if (is_duplicate_prev || is_duplicate) {
            struct list_head *const node_deleting = node_distinct->next;
            node_distinct->next = node_distinct->next->next;
            q_delete(node_deleting);
        } else {
            node_distinct = node_distinct->next;
        }
        is_duplicate_prev = is_duplicate;
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *node = head;
    while (true) {
        struct list_head *const next_orig = node->next;
        node->next = node->prev;
        node->prev = next_orig;
        if (next_orig == head)
            break;
        node = next_orig;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (head->prev == head->next)
        return;
    LIST_HEAD(temp_head_reverser);
    LIST_HEAD(temp_head_result);
    while (true) {
        struct list_head *node_last = head->next;
        for (int rem = k - 1; rem && node_last != head; --rem)
            node_last = node_last->next;
        if (node_last != head) {
            list_cut_position(&temp_head_reverser, head, node_last);
            q_reverse(&temp_head_reverser);
            list_splice_tail_init(&temp_head_reverser, &temp_head_result);
        } else {
            list_splice(&temp_head_result, head);
            break;
        }
    }
}

static void q_merge2(struct list_head *source,
                     struct list_head *target,
                     bool descend)
{
    if (!target || !source || list_empty(source))
        return;
    struct list_head *target_walker = target->next;
    while (target_walker != target && !list_empty(source)) {
        int ret_cmp = strcmp(list_entry(target_walker, element_t, list)->value,
                             list_entry(source->next, element_t, list)->value);
        if (descend ? (ret_cmp < 0) : (ret_cmp > 0)) {
            list_move_tail(source->next, target_walker);
        } else {
            target_walker = target_walker->next;
        }
    }
    list_splice_tail_init(source, target);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || head->prev == head->next)
        return;
    LIST_HEAD(temp_head);
    list_cut_position(&temp_head, head, q_get_mid(head));
    q_sort(&temp_head, descend);
    q_sort(head, descend);
    q_merge2(&temp_head, head, descend);
}

int q_monotone(struct list_head *head, bool descend)
{
    if (!head || head->next == head->prev) {
        return 0;
    }
    int count = 1;
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    for (struct list_head *walker = head->prev; walker->prev != head;) {
        const int ret_cmp =
            strcmp(list_entry(walker, element_t, list)->value,
                   list_entry(walker->prev, element_t, list)->value);
        if (descend ? (ret_cmp > 0) : (ret_cmp < 0)) {
            // "Remove" means not to free
            list_del_init(walker->prev);
        } else {
            ++count;
            walker = walker->prev;
        }
    }
    return count;
}

/* Remove every node which has a node with a strictly lower value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    return q_monotone(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_monotone(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head)
        return -1;
    if (list_empty(head))
        return 0;
    queue_contex_t *const target_context =
        list_entry(head->next, queue_contex_t, chain);
    // Need to skip the first queue, so we cannot use list_for_each_entry.
    for (struct list_head *current_node = head->next->next;
         current_node != head; current_node = current_node->next) {
        queue_contex_t *current_context =
            list_entry(current_node, queue_contex_t, chain);
        q_merge2(current_context->q, target_context->q, descend);
        // Set to null queue
        current_context->q = NULL;
        target_context->size += current_context->size;
        current_context->size = 0;
    }
    return target_context->size;
}

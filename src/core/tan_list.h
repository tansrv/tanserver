/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_LIST_H
#define TAN_LIST_H


/*
 * example:
 *
 * typedef struct {
 *     int              age;
 *     tan_list_node_t  node;
 * } student_t;
 *
 * tan_list_node_t  students;
 * tan_list_init(&students);
 *
 * student_t  Jack;
 * Jack.age = 21;
 *
 * tan_list_push_back(&students, &Jack.node);
 *
 * if (!tan_list_empty(&students)) {
 *
 *     student_t *first = tan_list_get_data(tan_list_first(&students),
 *                                          student_t, node);
 *
 *     printf("%d\n", first->age);
 * }
 */


typedef struct tan_list_node_s {
    struct tan_list_node_s  *prev;
    struct tan_list_node_s  *next;
} tan_list_node_t;


#define tan_list_init(sentinel)                                             \
    (sentinel)->prev = sentinel;                                            \
    (sentinel)->next = sentinel

#define tan_list_empty(sentinel)                                            \
    ((sentinel)->next == sentinel)

#define tan_list_push_back(sentinel, ele)                                   \
    (ele)->prev = (sentinel)->prev;                                         \
    (ele)->prev->next = ele;                                                \
    (sentinel)->prev = ele;                                                 \
    (ele)->next = sentinel

#define tan_list_first(sentinel)                                            \
    (sentinel)->next

#define tan_list_remove(ele)                                                \
    (ele)->next->prev = (ele)->prev;                                        \
    (ele)->prev->next = (ele)->next

/* Source code from Nginx - ngx_queue_data()  */
#define tan_list_get_data(ptr, type, member)                                \
    (type *)((char *)ptr - offsetof(type, member))

#define tan_list_for_each(sentinel, data, tmp, type, member)                \
    for (data = tan_list_get_data(tan_list_first(sentinel), type, member);  \
         tmp = data->member.next, &data->member != sentinel;                \
         data = tan_list_get_data(tmp, type, member))


#endif /* TAN_LIST_H */

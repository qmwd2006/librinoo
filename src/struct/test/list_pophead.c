/**
 * @file   list_pophead.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2011
 * @date   Tue May 11 10:02:07 2010
 *
 * @brief  list_pophead unit test
 *
 *
 */

#include	"rinoo/rinoo.h"

int		cmp_func(void *node1, void *node2)
{
  (void) node1;
  (void) node2;
  return (1);
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int		main()
{
  t_list	*list;

  list = list_create(cmp_func);
  XTEST(list != NULL);
  XTEST(list_add(list, INT_TO_PTR(42), NULL) != NULL);
  XTEST(list_add(list, INT_TO_PTR(43), NULL) != NULL);
  XTEST(list_add(list, INT_TO_PTR(44), NULL) != NULL);
  XTEST(list_add(list, INT_TO_PTR(45), NULL) != NULL);
  XTEST(list->head != NULL);
  XTEST(list->tail != NULL);
  XTEST(list->size == 4);
  XTEST(list->head->node == INT_TO_PTR(42));
  XTEST(list->tail->node == INT_TO_PTR(45));
  XTEST(list_pophead(list) == INT_TO_PTR(42));
  XTEST(list->head != NULL);
  XTEST(list->head->node == INT_TO_PTR(43));
  XTEST(list->tail != NULL);
  XTEST(list->tail->node == INT_TO_PTR(45));
  XTEST(list->size == 3);
  XTEST(list_pophead(list) == INT_TO_PTR(43));
  XTEST(list->head != NULL);
  XTEST(list->head->node == INT_TO_PTR(44));
  XTEST(list->tail != NULL);
  XTEST(list->tail->node == INT_TO_PTR(45));
  XTEST(list->size == 2);
  XTEST(list_pophead(list) == INT_TO_PTR(44));
  XTEST(list->head != NULL);
  XTEST(list->head == list->tail);
  XTEST(list->head->node == INT_TO_PTR(45));
  XTEST(list->size == 1);
  XTEST(list_pophead(list) == INT_TO_PTR(45));
  XTEST(list->head == NULL);
  XTEST(list->tail == NULL);
  XTEST(list->size == 0);
  list_destroy(list);
  XPASS();
}

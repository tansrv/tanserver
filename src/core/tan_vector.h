/* The MIT License
   Copyright (c) 2008, by Attractive Chaos <attractor@live.co.uk>
   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:
   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.  */


#ifndef TAN_VECTOR_H
#define TAN_VECTOR_H


/*
 * example:
 *
 * tan_vector_t(numbers, int);
 *
 * tan_vector_numbers_t  numbers;
 * tan_vector_init(numbers);
 *
 * tan_vector_push_back(numbers, int, 20);
 * tan_vector_push_back(numbers, int, 30);
 *
 * for (int k = 0; k < numbers.size; ++k)
 *     printf("%d\n", numbers.vec[k]);
 */


#define tan_vector_t(name, type)                                                \
    typedef struct {                                                            \
        type   *vec;                                                            \
        size_t  size, m;                                                        \
    } tan_vector_##name##_t

#define tan_vector_init(name)                                                   \
    (name).size = (name).m = 0, (name).vec = NULL

#define tan_vector_push_back(name, type, data)                                  \
    do {                                                                        \
        if ((name).size == (name).m) {                                          \
            (name).m = (name).m ? (name).m<<1 : 2;                              \
            (name).vec = (type *)realloc((name).vec, sizeof(type) * (name).m);  \
        }                                                                       \
        (name).vec[(name).size++] = data;                                       \
    } while (0)


#endif /* TAN_VECTOR_H */

#ifndef COMMIT_SLAB_H
#define COMMIT_SLAB_H

/*
 * define_commit_slab(slabname, elemtype) creates boilerplate code to define
 * a new struct (struct slabname) that is used to associate a piece of data
 * of elemtype to commits, and a few functions to use that struct.
 *
 * After including this header file, using:
 *
 * define_commit_slab(indegee, int);
 *
 * will let you call the following functions:
 *
 * - int *indegree_at(struct indegree *, struct commit *);
 *
 *   This function locates the data associated with the given commit in
 *   the indegree slab, and returns the pointer to it.
 *
 * - void init_indegree(struct indegree *, int stride);
 *
 *   Initializes the indegree slab that associates an array of integers
 *   to each commit. 'stride' specifies how big each array is.
 */

/* allocate ~512kB at once, allowing for malloc overhead */
#ifndef COMMIT_SLAB_SIZE
#define COMMIT_SLAB_SIZE (512*1024-32)
#endif

#define define_commit_slab(slabname, elemtype) 				\
									\
struct slabname {							\
	unsigned slab_size;						\
	unsigned stride;						\
	unsigned slab_count;						\
	elemtype **slab;						\
};									\
static int stat_ ##slabname## realloc;					\
									\
static void init_ ##slabname## _with_stride(struct slabname *s,		\
					    unsigned stride)		\
{									\
	unsigned int elem_size;						\
	if (!stride)							\
		stride = 1;						\
	s->stride = stride;						\
	elem_size = sizeof(struct slabname) * stride;			\
	s->slab_size = COMMIT_SLAB_SIZE / elem_size;			\
	s->slab_count = 0;						\
	s->slab = NULL;							\
}									\
									\
static void init_ ##slabname(struct slabname *s)			\
{									\
	init_ ##slabname## _with_stride(s, 1);				\
}									\
									\
static void clear_ ##slabname(struct slabname *s)			\
{									\
	int i;								\
	for (i = 0; i < s->slab_count; i++)				\
		free(s->slab[i]);					\
	s->slab_count = 0;						\
	free(s->slab);							\
	s->slab = NULL;							\
}									\
									\
static elemtype *slabname## _at(struct slabname *s,			\
				const struct commit *c)			\
{									\
	int nth_slab, nth_slot, ix;					\
									\
	ix = c->index * s->stride;					\
	nth_slab = ix / s->slab_size;					\
	nth_slot = ix % s->slab_size;					\
									\
	if (s->slab_count <= nth_slab) {				\
		int i;							\
		s->slab = xrealloc(s->slab,				\
				   (nth_slab + 1) * sizeof(s->slab));	\
		stat_ ##slabname## realloc++;				\
		for (i = s->slab_count; i <= nth_slab; i++)		\
			s->slab[i] = NULL;				\
		s->slab_count = nth_slab + 1;				\
	}								\
	if (!s->slab[nth_slab])						\
		s->slab[nth_slab] = xcalloc(s->slab_size,		\
					    sizeof(**s->slab));		\
	return &s->slab[nth_slab][nth_slot];				\
}									\
									\
static int stat_ ##slabname## realloc

#endif /* COMMIT_SLAB_H */

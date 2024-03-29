/* C routines for NoPotFF.py */

#include "MMTK/universe.h"
#include "MMTK/forcefield.h"
#include "MMTK/forcefield_private.h"
#include <math.h>

/* This function does the actual energy (and gradient) calculation.
   Everything else is just bookkeeping. */
static void
NoPot_evaluator(PyFFEnergyTermObject *self,
		   PyFFEvaluatorObject *eval,
		   energy_spec *input,
		   energy_data *energy)
     /* The four parameters are pointers to structures that are
	defined in MMTK/forcefield.h.
	PyFFEnergyTermObject: All data relevant to this particular
                              energy term.
        PyFFEvaluatorObject:  Data referring to the global energy
                              evaluation process, e.g. parallelization
                              options. Not used here.
        energy_spec:          Input parameters for this routine, i.e.
                              atom positions and parallelization parameters.
        energy_data:          Storage for the results (energy terms,
                              gradients, second derivatives).
     */
{
  vector3 *coordinates = (vector3 *)input->coordinates->data;
  vector3 *g;
  int atom_index1 = (int)self->param[0];  /* atom index */
  int atom_index2 = (int)self->param[1];  /* atom index */

/*
  printf("%i %i %i %i %i %i \n",atom_index1,atom_index2,atom_index3,atom_index4,atom_index5,atom_index6);
*/

  double x1 = coordinates[atom_index1][0];
  double y1 = coordinates[atom_index1][1];
  double z1 = coordinates[atom_index1][2];
  
  double x2 = coordinates[atom_index2][0];
  double y2 = coordinates[atom_index2][1];
  double z2 = coordinates[atom_index2][2];

  /* energy_terms is an array because each routine could compute
     several terms that should logically be kept apart. For example,
     a single routine calculates Lennard-Jones and electrostatic interactions
     in a single iteration over the nonbonded list. The separation of
     terms is only done for the benefit of user code (universe.energyTerms())
     returns the value of each term separately), the total energy is
     always the sum of all terms. Here we have only one energy term. */
 
  // unit convertion to MMTK units
  double e;
  double gr[6];
  double rvec[3];

  e = 0.0;
  
  gr[0]=0.0;
  gr[1]=0.0;
  gr[2]=0.0;
  gr[3]=0.0;
  gr[4]=0.0;
  gr[5]=0.0;

  energy->energy_terms[self->index] = e;
  /* If only the energy is asked for, stop here. */
  if (energy->gradients == NULL)
    return;
  
  /* Add the gradient contribution to the global gradient array.
     It would be a serious error to use '=' instead of '+=' here,
     in that case all previously calculated forces would be erased. */

  g = (vector3 *)((PyArrayObject*)energy->gradients)->data;


  g[atom_index1][0]+=gr[0];
  g[atom_index1][1]+=gr[1];
  g[atom_index1][2]+=gr[2];
 
  g[atom_index2][0]+=gr[3];
  g[atom_index2][1]+=gr[4];
  g[atom_index2][2]+=gr[5];

}

/* A utility function that allocates memory for a copy of a string */
static char *
allocstring(char *string)
{
  char *memory = (char *)malloc(strlen(string)+1);
  if (memory != NULL)
    strcpy(memory, string);
  return memory;
}

/* The next function is meant to be called from Python. It creates the
   energy term object at the C level and stores all the parameters in
   there in a form that is convient to access for the C routine above.
   This is the routine that is imported into and called by the Python
   module, HeHeFF.py. */
static PyObject *
NoPotTerm(PyObject *dummy, PyObject *args)
{
  PyFFEnergyTermObject *self;
  int atom_index1;
  int atom_index2;

  /* Create a new energy term object and return if the creation fails. */
  self = PyFFEnergyTerm_New();
  if (self == NULL)
    return NULL;
  /* Convert the parameters to C data types. */
  if (!PyArg_ParseTuple(args, "O!ii",
			&PyUniverseSpec_Type, &self->universe_spec,
			&atom_index1, &atom_index2))
    return NULL;
  /* We keep a reference to the universe_spec in the newly created
     energy term object, so we have to increase the reference count. */
  Py_INCREF(self->universe_spec);
  /* A pointer to the evaluation routine. */
  self->eval_func = NoPot_evaluator;
  /* The name of the energy term object. */
  self->evaluator_name = "NoPot";
  /* The names of the individual energy terms - just one here. */
  self->term_names[0] = allocstring("NoPot");
  if (self->term_names[0] == NULL)
    return PyErr_NoMemory();
  self->nterms = 1;
  /* self->param is a storage area for parameters. Note that there
     are only 40 slots (double) there, if you need more space, you can use
     self->data, an array for up to 40 Python object pointers. */
  self->param[0] = (double) atom_index1;
  self->param[1] = (double) atom_index2;
//  self->param[1] = (double) atom_index2;
  /* Return the energy term object. */
  return (PyObject *)self;
}

/* This is a list of all Python-callable functions defined in this
   module. Each list entry consists of the name of the function object
   in the module, the C routine that implements it, and a "1" signalling
   new-style parameter passing conventions (only veterans care about the
   alternatives). The list is terminated by a NULL entry. */
static PyMethodDef functions[] = {
  {"NoPotTerm", NoPotTerm, 1},
  {NULL, NULL}		/* sentinel */
};


/* The initialization function for the module. This is the only function
   that must be publicly visible, everything else should be declared
   static to prevent name clashes with other modules. The name of this
   function must be "init" followed by the module name. */
DL_EXPORT(void)
initMMTK_NoPot(void)
{
  PyObject *m;

  /* Create the module and add the functions. */
  m = Py_InitModule("MMTK_NoPot", functions);

  /* Import the array module. */
#ifdef import_array
  import_array();
#endif

  /* Import MMTK modules. */
  import_MMTK_universe();
  import_MMTK_forcefield();

  /* Check for errors. */
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module MMTK_NoPot");
}

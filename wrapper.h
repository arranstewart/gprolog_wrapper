
#ifndef WRAPPER
#define WRAPPER

#include <string>

// TODO: shift definitions into header once more complete.
struct atom;
struct term;
struct variable;
struct nil;

// atom func decls

std::string atom_to_str(atom at);
atom term_to_atom(term t);
atom find_atom(const char * atom_name);

template<typename T> 
  atom mk_atom(T);

template<>
  atom mk_atom<const char *>(const char *str);

template<>
  atom mk_atom<nil>(nil);

// term func decls

template<typename T> 
  term mk_term(T);

template<>
term mk_term<long>(long l);

template<>
term mk_term<int>(int l);

template<>
term mk_term<double>(double d);

template<>
term mk_term<atom>(atom a);

/** usage:
 *    term my_var_term = mk_term( variable { } );
 * variable is just a phantom/tag.
 */
template<>
term mk_term<variable>(variable v);


#endif
// WRAPPER

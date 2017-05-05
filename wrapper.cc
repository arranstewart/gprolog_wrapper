
#include "wrapper.h"

#include <gprolog.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <functional>
#include <exception>

using std::string;
using std::cout;
using std::endl;
using std::vector;

// set up #defines specifying the gprolog version you're using.
// if 1.3.0 (used on ubuntu) we set up some #defines so current (1.4)
// function names and types still work.
#define GPROLOG_VERSION_1_3_0 

#ifdef GPROLOG_VERSION_1_3_0

#define PlBool                  Bool
#define Pl_Start_Prolog         Start_Prolog
#define Pl_Find_Atom            Find_Atom
#define Pl_Start_Prolog         Start_Prolog 
#define Pl_Stop_Prolog          Stop_Prolog 
#define PL_TRUE                 TRUE
#define PL_FALSE                FALSE
#define Pl_Mk_Variable          Mk_Variable 
#define Pl_Mk_Atom              Mk_Atom 
#define Pl_Rd_Atom              Rd_Atom   
//  Pl_Query_Next_Solution is actually fine as is.
#define Pl_Un_Proper_List_Check Un_Proper_List_Check
#define Pl_Write                Write_Simple
#define Pl_Mk_Integer           Mk_Integer
#define Pl_Create_Atom          Create_Atom
#define Pl_Mk_Float             Mk_Float
#define Pl_Atom_Nil             Atom_Nil
#define Pl_Atom_Name            Atom_Name 
#define Pl_Rd_Atom_Check        Rd_Atom_Check     


#endif
// GPROLOG_VERSION_1_3_0


// wrappers around the C "types".
// it's guaranteed (since they have no vtables) that a
// pointer to them points to the same spot as the first member.
//    It's *not* guaranteed that the struct as a whole is of the same
// size (compilers are at liberty to add packing, for instane).
//    So which does reinterpret_cast() between arrays of the two,
// is not at all guaranteed to work. (Tho it does, currently, for gcc
// and clang.)

struct atom {
  int atom_id;
};

struct term {
  PlTerm the_term;
};

// empty structs. Just used as tags/phantoms to make creation
// of vectors of terms more readable.

struct variable {
};

struct nil {
};


// atom functions
/////////////////

string atom_to_str(atom at) {
  string res  {  Pl_Atom_Name(at.atom_id)  };
  return res;
}

// this will probably result in some kind of (gprolog) error
// if the term is not, indeed, unified with an atom.
atom term_to_atom(term t) {
  atom myAt { Pl_Rd_Atom_Check(t.the_term) } ;
  return myAt;
}

template<typename T> 
inline atom mk_atom(T) {
  static_assert(sizeof(T) != sizeof(T), "func mk_atom must be specialized for this type");
}

// construct an atom from a char *.
template<>
inline atom mk_atom<const char *>(const char *str) {
  // mersion 1.3 has a char *. If gprolog does indeed guarantee
  // not to change the char *, the const_cast should be fine.
  int at_id = Pl_Create_Atom(
    const_cast<char *>(
      str
    )
  );
  return { at_id };
}

// get the nil atom
template<>
inline atom mk_atom<nil>(nil) {
  return find_atom("[]");
};



inline atom find_atom(const char * atom_name) {
  // The docco seems to promise that this will, in fact, be 
  // respected as const.
  int my_atom_id = Find_Atom(
    const_cast<char *>(
      atom_name
    )
  );
  (my_atom_id != -1) || ({
                printf("couldn't find an atom '%s'!!!\n", atom_name);
                exit(EXIT_FAILURE);
                1;
             });
  return { my_atom_id };
}

// term functions

template<typename T> 
inline term mk_term(T) {
  static_assert(sizeof(T) != sizeof(T), "func mk_term must be specialized for this type");
}

template<>
inline term mk_term<long>(long l) {
  return { Mk_Integer(l) };
}  

template<>
inline term mk_term<int>(int l) {
  return mk_term( (long) l); 
}  

template<>
inline term mk_term<double>(double d) {
  return { Pl_Mk_Float( d ) };
}  

template<>
inline term mk_term<atom>(atom a) {
  return { Pl_Mk_Atom( a.atom_id ) };
}

/** usage:
 *    term my_var_term = mk_term( variable { } );
 *
 * variable is really just a tag, a phantom type, to show
 * what we want to make.
 */
template<>
inline term mk_term<variable>(variable v) {
  return { Pl_Mk_Variable() };
}


// API for queries & programs


class prolog_exception : public std::runtime_error {
public:
  prolog_exception(string str) 
    : runtime_error( str )
    {}
};

class oneSoln {
public:
  oneSoln(const string & functor_name, std::initializer_list<term> args)
    : m_functor_name(functor_name), 
      m_args(args)
    {}

  oneSoln(const string & functor_name, vector<term> args)
    : m_functor_name(functor_name), 
      m_args(args)
    {}

  term arg(int i) {
    return m_args.at(i);
  }

  /** call the functor with args, and just get one solution.
   * returns result - 
   *    PL_FAILURE on failure,
   *    PL_SUCCESS on success, and unifications will have been done, or
   *    it'll die on PL_EXCEPTION, but print the exception.
   */ 
  int operator()(bool verbose=false) {
    // we're actually going to fiddle with the contents.
    // ah well.
    term * args_ = const_cast<term *>(m_args.data());
    PlTerm * pl_args = reinterpret_cast<PlTerm *>( args_);
   
    atom functor = find_atom(m_functor_name.c_str() ); 
    int result = Pl_Query_Call(functor.atom_id, m_args.size(), pl_args);

    if (result == PL_EXCEPTION) {
      PlTerm exception = Pl_Get_Exception();
      Pl_Write(exception);
      cout << "exception calling " << m_functor_name << endl;

      std::ostringstream oss;
      oss << "exception calling " << m_functor_name << endl;
      throw prolog_exception( oss.str() );
    }
    if (verbose) {
      cout << "\nresult of call to " << m_functor_name << "/"
           << m_args.size() << " was: " << result << endl;
    }
    return  result;
  }

private:
  string m_functor_name;
  const vector<term> m_args; 
};


class call {
public:
  call(std::function<void()> func)
    : m_call(func)
    {}   

private:  
  void operator()() {
    m_call();
  }

  std::function<void()> m_call;

  friend class query;  
};

class query {
public:
  query()
    : calls()
    {}

  query & add(call c) {
    calls.push_back(c);
    return *this;
  }

private:
  void operator()() {
    Pl_Query_Begin(PL_TRUE);
    for( auto it = calls.begin(); it != calls.end(); it++ ) {
      call c = (*it);
      c();
    }
    // TODO - provide version for queries where we expect multiple
    // solns.
    //Pl_Query_End(PL_RECOVER);
    Pl_Query_End(PL_CUT);
  }

  vector<call> calls;
  friend class program;
};

class program {
public:
  program()
    : queries(), hasBegun(false)
    {}

  program & add(query q) {
    queries.push_back(q);
    return *this;
  }

  void operator()() {
    begin();
    for( auto it = queries.begin(); it != queries.end(); it++ ) {
      query q = (*it);
      q();
    }
    Pl_Stop_Prolog();
  }

  ~program() {
    if (hasBegun) {
      end(); }
  }

private:
  vector<query> queries;

  void begin() {
    Pl_Start_Prolog(0, NULL);
    hasBegun = true; 
  }

  void end() {
    Pl_Stop_Prolog();
    hasBegun = false;
  }

  bool hasBegun;

};

void example() {

  query q1;

  q1.add( 
    call { []() -> void {
      const auto nil_at = mk_atom( nil {} );
      cout << "try write nil" << endl;
      int res = oneSoln {"write",{ mk_term(nil_at)}} (true);
    }} 
  ).add(
    call { []() -> void {
      int res;
      const auto nil_at = mk_atom( nil {} );
      oneSoln mycall("length", {
        mk_term(nil_at), mk_term(variable{}) 
      });
      res = mycall(true);

      oneSoln writeArg("write", { mycall.arg(1) } ); 
      res = writeArg(true);

    }}
  );

  program p {};
  p.add(q1);
  p();

}







// RUN: %clang_cc1 -std=c++11 -verify -fsyntax-only %s
// RUN: %clang_cc1 -std=c++1y %s -verify -DCXX1Y

// Explicit member declarations behave as in C++11.

namespace n3323_example {

  template <class T> class zero_init {
  public:
    zero_init() : val(static_cast<T>(0)) {}
    zero_init(T val) : val(val) {}

    operator T &() { return val; }     //@13
    operator T() const { return val; } //@14

  private:
    T val;
  };

  void Delete() {
    zero_init<int *> p;
    p = new int(7);
    delete p; //@23
    delete (p + 0);
    delete + p;
  }

  void Switch() {
    zero_init<int> i;
    i = 7;
    switch (i) {} // @31
    switch (i + 0) {}
    switch (+i) {}
  }
}

#ifdef CXX1Y
#else
//expected-error@23 {{ambiguous conversion of delete expression of type 'zero_init<int *>' to a pointer}}
//expected-note@13 {{conversion to pointer type 'int *'}}
//expected-note@14 {{conversion to pointer type 'int *'}}
//expected-error@31 {{multiple conversions from switch condition type 'zero_init<int>' to an integral or enumeration type}}
//expected-note@13 {{conversion to integral type 'int'}}
//expected-note@14 {{conversion to integral type 'int'}}
#endif

namespace extended_examples {

  struct A0 {
    operator int();      // matching and viable
  };

  struct A1 {
    operator int() &&;   // matching and not viable
  };

  struct A2 {
    operator float();    // not matching
  };

  struct A3 {
    template<typename T> operator T();  // not matching (ambiguous anyway)
  };

  struct A4 {
    template<typename T> operator int();  // not matching (ambiguous anyway)
  };

  struct B1 {
    operator int() &&;  // @70
    operator int();     // @71  -- duplicate declaration with different qualifier is not allowed
  };

  struct B2 {
    operator int() &&;  // matching but not viable
    operator float();   // not matching
  };

  void foo(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, B2 b2) {
    switch (a0) {}
    switch (a1) {}     // @81 -- fails for different reasons
    switch (a2) {}     // @82
    switch (a3) {}     // @83
    switch (a4) {}     // @84
    switch (b2) {}     // @85 -- fails for different reasons
  }
}

//expected-error@71 {{cannot overload a member function without a ref-qualifier with a member function with ref-qualifier '&&'}}
//expected-note@70 {{previous declaration is here}}
//expected-error@82 {{statement requires expression of integer type ('extended_examples::A2' invalid)}}
//expected-error@83 {{statement requires expression of integer type ('extended_examples::A3' invalid)}}
//expected-error@84 {{statement requires expression of integer type ('extended_examples::A4' invalid)}}

#ifdef CXX1Y
//expected-error@81 {{statement requires expression of integer type ('extended_examples::A1' invalid)}}
//expected-error@85 {{statement requires expression of integer type ('extended_examples::B2' invalid)}}
#else
//expected-error@81 {{cannot initialize object parameter of type 'extended_examples::A1' with an expression of type 'extended_examples::A1'}}
//expected-error@85 {{cannot initialize object parameter of type 'extended_examples::B2' with an expression of type 'extended_examples::B2'}}
#endif

namespace extended_examples_cxx1y {

  struct A1 {   // leads to viable match in C++1y, and no viable match in C++11
    operator int() &&;                  // matching but not viable
    template <typename T> operator T(); // In C++1y: matching and viable, since disambiguated by L.100
  };

  struct A2 {   // leads to ambiguity in C++1y, and no viable match in C++11
    operator int() &&;                    // matching but not viable
    template <typename T> operator int(); // In C++1y: matching but ambiguous (disambiguated by L.105).
  };

  struct B1 {    // leads to one viable match in both cases
    operator int();                  // matching and viable
    template <typename T> operator T(); // In C++1y: matching and viable, since disambiguated by L.110
  };

  struct B2 {    // leads to one viable match in both cases
    operator int();                  // matching and viable
    template <typename T> operator int(); // In C++1y: matching but ambiguous, since disambiguated by L.115
  };

  struct C {    // leads to no match in both cases
    operator float();                  // not matching
    template <typename T> operator T(); // In C++1y: not matching, nor viable.
  };

  struct D {   // leads to viable match in C++1y, and no viable match in C++11
    operator int() &&;                  // matching but not viable
    operator float();                   // not matching
    template <typename T> operator T(); // In C++1y: matching and viable, since disambiguated by L.125
  };


  void foo(A1 a1, A2 a2, B1 b1, B2 b2, C c, D d) {
    switch (a1) {} // @138 --  should presumably call templated conversion operator to convert to int.
    switch (a2) {} // @139
    switch (b1) {}
    switch (b2) {}
    switch (c) {}  // @142
    switch (d) {}  // @143
  }
}

//expected-error@142 {{statement requires expression of integer type ('extended_examples_cxx1y::C' invalid)}}

#ifdef CXX1Y
//expected-error@139 {{statement requires expression of integer type ('extended_examples_cxx1y::A2' invalid)}}
#else
//expected-error@138 {{cannot initialize object parameter of type 'extended_examples_cxx1y::A1' with an expression of type 'extended_examples_cxx1y::A1'}}
//expected-error@139 {{cannot initialize object parameter of type 'extended_examples_cxx1y::A2' with an expression of type 'extended_examples_cxx1y::A2'}}
//expected-error@143 {{cannot initialize object parameter of type 'extended_examples_cxx1y::D' with an expression of type 'extended_examples_cxx1y::D'}}
#endif

// FIXME: Extend with more examples, including [expr.const] and [expr.new].

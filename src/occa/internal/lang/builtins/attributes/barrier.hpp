#ifndef OCCA_INTERNAL_LANG_BUILTINS_ATTRIBUTES_BARRIER_HEADER
#define OCCA_INTERNAL_LANG_BUILTINS_ATTRIBUTES_BARRIER_HEADER

#include <occa/internal/lang/attribute.hpp>

namespace occa {
  namespace lang {
    namespace attributes {
      class barrier : public attribute_t {
      public:
        barrier();

        virtual const std::string& name() const;

        virtual bool forStatementType(const int sType) const;

        virtual bool isValid(const attributeToken_t &attr) const;
      };
    }
  }
}

#endif

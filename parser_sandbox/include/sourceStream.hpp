/* The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 David Medina and Tim Warburton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 */
#ifndef OCCA_PARSER_SOURCETREAM_HEADER2
#define OCCA_PARSER_SOURCETREAM_HEADER2

#include "occa/tools/io.hpp"

#include "file.hpp"
#include "tokenStream.hpp"

namespace occa {
  namespace lang {
    typedef std::vector<token_t*> tokenVector;
    typedef std::vector<fileOrigin> originVector;

    int getEncodingType(const std::string &str);
    int getCharacterEncoding(const std::string &str);
    int getStringEncoding(const std::string &str);

    class sourceStream : public tokenStream {
    public:
      fileOrigin origin;
      filePosition &fp;
      originVector stack;
      std::vector<originVector> sourceStack;

      sourceStream(const char *root);
      sourceStream(file_t *file_,
                   const char *root);

      sourceStream(const sourceStream &stream);
      sourceStream& operator = (const sourceStream &stream);

      virtual void preprint(std::ostream &out);
      virtual void postprint(std::ostream &out);

      void setLine(const int line);

      bool isEmpty();
      void pushSource(const bool fromInclude,
                      file_t *file,
                      const filePosition &position);
      void popSource();

      void push();
      void pop(const bool rewind = false);
      void popAndRewind();
      std::string str();

      void countSkippedLines();

      void skipTo(const char delimiter);
      void skipTo(const char *delimiters);
      void skipFrom(const char *delimiters);

      void skipWhitespace();

      int peek();
      int shallowPeek();
      int peekForIdentifier();
      int peekForOperator();
      int peekForHeader();

      void getIdentifier(std::string &value);
      void getString(std::string &value,
                     const int encoding = 0);
      void getRawString(std::string &value);

      int skipLineCommentAndPeek();
      int skipBlockCommentAndPeek();

      virtual token_t* getToken();
      token_t* getIdentifierToken();
      token_t* getPrimitiveToken();
      token_t* getOperatorToken();
      token_t* getStringToken(const int encoding);
      token_t* getCharToken(const int encoding);
      token_t* getHeaderToken();
    };
  }
}

#endif
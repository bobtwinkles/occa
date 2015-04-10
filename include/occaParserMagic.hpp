#ifndef OCCA_PARSER_MAGIC_HEADER
#define OCCA_PARSER_MAGIC_HEADER

#include "occaParser.hpp"

namespace occa {
  namespace parserNS {
    class parserBase;
    class viInfo_t;
    class infoDB_t;

    typedef std::map<statement*, int> smntInfoMap_t;
    typedef smntInfoMap_t::iterator   smntInfoIterator;

    typedef std::map<varInfo*, viInfo_t*> viInfoMap_t_;
    typedef viInfoMap_t_::iterator        viInfoIterator;

    namespace viType {
      static const int isUseless        = (1 << 0);
      static const int isAVariable      = (1 << 1);
      static const int isAnIterator     = (1 << 2);
      static const int isConstant       = (1 << 3);
      static const int isComplex        = (1 << 4);

      std::string infoToStr(const int info);
    };

    namespace analyzeInfo {
      // Statement Info
      static const int isExecuted  = (1 << 0);
      static const int isIgnored   = (1 << 1);
      static const int schrodinger = (isExecuted | isIgnored); // hehe

      static const int noLCD       = (1 << 2);

      // Return info
      static const int didntChange = 0;
      static const int changed     = 1;
    };

    class atomInfo_t {
    public:
      infoDB_t *db;

      int info;
      typeHolder constValue;
      expNode *exp;
      varInfo *var;

      atomInfo_t(infoDB_t *db_ = NULL);
      atomInfo_t(const atomInfo_t &ai);

      atomInfo_t& operator = (const atomInfo_t &ai);

      bool operator == (const std::string &str);
      bool operator == (expNode &e);

      void setDB(infoDB_t *db_);

      void load(expNode &e);
      void load(varInfo &var_);
      void load(const std::string &s);

      bool expandValue();
      bool expandValue(expNode &e);
      bool expandValue(expNode *&expRoot, varInfo &v);

      void saveTo(expNode &e, const int leafPos = 0);

      bool isComplex();

      std::string getInfoStr();

      friend std::ostream& operator << (std::ostream &out, atomInfo_t &info);
    };

    class valueInfo_t {
    public:
      infoDB_t *db;

      int info;
      int indices;
      atomInfo_t value;
      atomInfo_t *vars, *strides;

      valueInfo_t(infoDB_t *db_ = NULL);
      valueInfo_t(expNode &e, infoDB_t *db_ = NULL);

      valueInfo_t(const valueInfo_t &vi);
      valueInfo_t& operator = (const valueInfo_t &vi);

      void setDB(infoDB_t *db_);

      void allocVS(const int count);

      bool isUseless();
      bool isComplex();

      void load(expNode &e);
      void load(varInfo &var);
      void load(const std::string &s);

      void loadVS(expNode &e, const int pos);

      void sortIndices();
      static int qSortIndices(const void *a, const void *b);

      void mergeIndices();

      bool expandValues();

      void saveTo(expNode &e, const int leafPos = 0);
      void saveIndexTo(const int index, expNode &e, const int leafPos = 0);

      void update(expNode &op, expNode &e);

      int hasStride(const std::string &str);
      int hasStride(expNode &e);

      void add(const std::string &str);
      void add(expNode &e);

      void sub(const std::string &str);
      void sub(expNode &e);

      typeHolder constValue();
      varInfo& varValue();
      varInfo& var(const int pos);
      atomInfo_t& stride(const int pos);

      friend std::ostream& operator << (std::ostream &out, valueInfo_t &info);
    };

    class accessInfo_t {
    public:
      infoDB_t *db;
      statement *s;

      int dim;
      valueInfo_t value;
      valueInfo_t *dimIndices;

      accessInfo_t(infoDB_t *db_ = NULL);
      accessInfo_t(const accessInfo_t &ai);

      accessInfo_t& operator = (const accessInfo_t &ai);

      void setDB(infoDB_t *db_);

      void load(expNode &varNode);
      void load(const int brackets, expNode &bracketNode);

      bool conflictsWith(accessInfo_t &ai);

      friend std::ostream& operator << (std::ostream &out, accessInfo_t &info);
    };

    class iteratorInfo_t {
    public:
      infoDB_t *db;

      statement *s;
      valueInfo_t start, end, stride;

      iteratorInfo_t(infoDB_t *db_ = NULL);

      void setDB(infoDB_t *db_);

      friend std::ostream& operator << (std::ostream &out, iteratorInfo_t &info);
    };

    class viInfo_t {
    public:
      infoDB_t *db;

      int info;
      valueInfo_t    valueInfo;
      iteratorInfo_t iteratorInfo;

      std::vector<accessInfo_t> reads, writes;

      static const int writeValue = (1 << 0);
      static const int readValue  = (1 << 1);

      viInfo_t(infoDB_t *db_ = NULL);

      void setDB(infoDB_t *db_);

      bool hasBeenInitialized();

      accessInfo_t& addWrite(expNode &varNode);
      accessInfo_t& addWrite(const int brackets, expNode &bracketNode);

      accessInfo_t& addRead(expNode &varNode);
      accessInfo_t& addRead(const int brackets, expNode &bracketNode);

      void updateValue(expNode &opNode, expNode &setNode);

      void checkComplexity();

      void checkLastInput(accessInfo_t &ai, const int inputType);

      friend std::ostream& operator << (std::ostream &out, viInfo_t &info);
    };

    class viInfoMap_t {
    public:
      infoDB_t *db;

      viInfoMap_t_ viMap;
      viInfo_t *anonVar; // Stores non-restrict variables

      viInfoMap_t(infoDB_t *db_);

      void setDB(infoDB_t *db_);

      void free();

      void add(varInfo &var);
      viInfo_t* has(varInfo &var);

      viInfo_t& operator [] (varInfo &var);
    };

    class infoDB_t {
    public:
      viInfoMap_t viInfoMap;
      smntInfoMap_t smntInfoMap;
      std::stack<int> smntInfoStack;

      infoDB_t();

      int& getSmntInfo();

      void add(varInfo &var);
      viInfo_t* has(varInfo &var);

      void enteringStatement(statement &s);
      void leavingStatement();

      viInfo_t& operator [] (varInfo &var);
    };

    class magician {
    public:
      parserBase &parser;
      statement &globalScope;
      varUsedMap_t &varUpdateMap;
      varUsedMap_t &varUsedMap;

      infoDB_t db;

      magician(parserBase &parser_);

      viInfoMap_t* currentViInfoMap();
      void pushMapStack();
      void popMapStack();

      static void castMagicOn(parserBase &parser_);

      void castMagic();
      void analyzeFunction(statement &fs);
      void analyzeStatement(statement &s);

      void analyzeEmbeddedStatements(statement &s);

      void analyzeDeclareStatement(expNode &e);
      void analyzeDeclareExpression(expNode &e, const int pos);

      void analyzeUpdateStatement(expNode &e);
      void analyzeUpdateExpression(expNode &e, const int pos);

      void analyzeForStatement(statement &s);
      void analyzeFortranForStatement(statement &s);

      void analyzeWhileStatement(statement &s);

      void analyzeIfStatement(statementNode *snStart, statementNode *snEnd);

      void analyzeSwitchStatement(statement &s);

      bool statementGuaranteesBreak(statement &s);

      bool variableIsUpdated(expNode &varNode);

      void addVariableWrite(expNode &varNode, expNode &opNode, expNode &setNode);
      void addVariableWrite(expNode &varNode,
                            expNode &opNode,
                            expNode &setNode,
                            const int brackets,
                            expNode &bracketNode);

      void addVariableRead(expNode &varNode);
      void addVariableRead(expNode &varNode,
                           const int brackets,
                           expNode &bracketNode);

      void addExpressionRead(expNode &e);
    };
  };
};

#endif

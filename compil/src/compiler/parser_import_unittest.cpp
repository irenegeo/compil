#include "parser_unittest.h"

#include "parser.h"

#include "gtest/gtest.h"

#include <iostream>


class ParserImportTests : public BaseParserTests 
{
public:

};

TEST_F(ParserImportTests, import)
{
    ASSERT_FALSE( parse(
        "import") );

    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());
    EXPECT_TRUE(checkErrorMessage(0, 1, 7, compil::Message::p_expectImportSource));
}

TEST_F(ParserImportTests, importSource)
{
    ASSERT_FALSE( parse(
        "import \"blah\"") );

    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());
    EXPECT_TRUE(checkErrorMessage(0, 1, 14, compil::Message::p_expectSemicolon));
}

TEST_F(ParserImportTests, importSourceClose)
{
    ASSERT_TRUE( parse(
        "import \"blah\";") );

    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());
    EXPECT_TRUE(checkWarningMessage(0, 1, 1, compil::Message::p_importWithoutSourceProvider));
}



class SourceProvider : public compil::ISourceProvider
{
public:
    virtual compil::SourceIdSPtr sourceId(const compil::SourceIdSPtr& pParent, const std::string& source)
    {
        if (source == "nonexistent")
            return compil::SourceIdSPtr();
            
        return compil::SourceIdSPtr(
            compil::SourceId::Builder()
                .set_value(source)
                .set_parent(pParent)
                .finalize());  
    }
    
    virtual StreamPtr openInputStream(const compil::SourceIdSPtr& pSourceId)
    {
        mSource = pSourceId->value();
        if (mSource == "import_nonexistent")
            return BaseParserTests::getInput("import \"nonexistent\";");
        if (mSource == "import_nonexistent")
            return BaseParserTests::getInput("import \"unopenable\";");
        if (mSource == "import_empty")
            return BaseParserTests::getInput("import \"empty\";");
        if (mSource == "import_circular")
            return BaseParserTests::getInput("import \"import_circular\";");
        if (mSource == "import_indirect_circular")
            return BaseParserTests::getInput("import \"import_indirect_circular2\";");
        if (mSource == "import_indirect_circular2")
            return BaseParserTests::getInput("import \"import_indirect_circular3\";");
        if (mSource == "import_indirect_circular3")
            return BaseParserTests::getInput("import \"import_indirect_circular\";");
        if (mSource == "import_error")
            return BaseParserTests::getInput("import \"import_error2\";");
        if (mSource == "import_error2")
            return BaseParserTests::getInput("import \"\"");
        if (mSource == "import_type")
            return BaseParserTests::getInput(
                "import \"import_type2\";\n"
                "import \"import_type3\";\n"
                "structure import_test { imported_enum2 ie2; imported_enum3 ie3; }");
        if (mSource == "import_type2")
            return BaseParserTests::getInput(
                "import \"import_type3\";\n"
                "enum imported_enum2 { value; }");
        if (mSource == "import_type3")
            return BaseParserTests::getInput("enum imported_enum3 { value; }");
        if (mSource == "empty")
            return BaseParserTests::getInput("");
        if (mSource == "unopenable")
            return StreamPtr();
            
        return StreamPtr();
    }
    
    virtual std::string getUniquePresentationString(const std::string& source)
    {
        return "";
    }
    
    std::string mSource;
};

TEST_F(ParserImportTests, importNonexistent)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_nonexistent").finalize();
    ASSERT_FALSE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());
    EXPECT_TRUE(checkErrorMessage(0, 1, 1, compil::Message::p_sourceNotFound));
    EXPECT_STREQ("import_nonexistent", pSourceProvider->mSource.c_str());
}

TEST_F(ParserImportTests, importUnopenable)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_unopenable").finalize();
    ASSERT_FALSE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());
    compil::Message message = mpMessageCollector->messages()[0];
    ASSERT_NE(message.sourceId(), mpSourceId);
    mpSourceId = message.sourceId();
    EXPECT_TRUE(checkErrorMessage(0, 0, 0, compil::Message::p_openSourceFailed));
    EXPECT_STREQ("import_unopenable", pSourceProvider->mSource.c_str());
}

TEST_F(ParserImportTests, importEmptySource)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_empty").finalize();
    ASSERT_TRUE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
    EXPECT_STREQ("empty", pSourceProvider->mSource.c_str());
}

TEST_F(ParserImportTests, importCircular)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_circular").finalize();
    ASSERT_TRUE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
}

TEST_F(ParserImportTests, importIndirectCircular)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_indirect_circular").finalize();
    ASSERT_TRUE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
}

TEST_F(ParserImportTests, importError)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_error").finalize();
    ASSERT_FALSE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
    ASSERT_EQ(1U, mpParser->mpMessageCollector->messages().size());

    compil::Message message = mpMessageCollector->messages()[0];
    ASSERT_NE(message.sourceId(), mpSourceId);
    mpSourceId = message.sourceId();
    EXPECT_TRUE(checkErrorMessage(0, 1, 10, compil::Message::p_expectSemicolon));
}

TEST_F(ParserImportTests, importType)
{
    boost::shared_ptr<SourceProvider> pSourceProvider(new SourceProvider());
    mpSourceId = compil::SourceId::Builder().set_value("import_type").finalize();
    ASSERT_TRUE( mpParser->parse(pSourceProvider, mpSourceId, mpModel) );
    EXPECT_TRUE(mpModel->mainDocument());
    EXPECT_EQ(mpSourceId, mpModel->mainDocument()->sourceId());
}
/*************************************************************************
> File Name       : QueryResult.h
> Brief           : 查询结果集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月22日  17时43分32秒
************************************************************************/
#pragma once
#include "MysqlTypeDef.h"
#include <vector>
#include "Field.h"

class ResultSet
{
public:
    ResultSet(ResultSet &&)            = delete;
    ResultSet &operator=(ResultSet &&) = delete;
    ResultSet(const ResultSet &)       = delete;
    ResultSet &operator=(ResultSet &)  = delete;

    ResultSet(MySqlResult *pResult, MySqlField *pFields, uint64_t rowCount, uint32_t fieldCount);
    ~ResultSet();

    bool NextRow();

    [[nodiscard]] uint64_t GetRowCount() const
    {
        return _rowCount;
    }

    [[nodiscard]] uint32_t GetFieldCount() const
    {
        return _fieldCount;
    }

    [[nodiscard]] Field *Fetch() const
    {
        return _pCurrentRow;
    }

    const Field &operator[](std::size_t index) const;

private:
    void CleanUp();

private:
    std::vector<QueryResultFieldMetadata> _fieldMetadata;
    uint64_t                              _rowCount;
    Field                                *_pCurrentRow;
    uint32_t                              _fieldCount;

    MySqlResult *_pResult;
    MySqlField  *_pFields;
};

class PreparedResultSet
{
public:
    PreparedResultSet(MySqlStmt *pStmt, MySqlResult *pResult, uint64_t rowCount, uint32_t fieldCount);
    ~PreparedResultSet();

    PreparedResultSet(PreparedResultSet const &right)            = delete;
    PreparedResultSet &operator=(PreparedResultSet const &right) = delete;
    PreparedResultSet(PreparedResultSet &&)                      = delete;
    PreparedResultSet &operator=(PreparedResultSet &&)           = delete;

    bool NextRow();

    [[nodiscard]] uint64_t GetRowCount() const
    {
        return _rowCount;
    }

    [[nodiscard]] uint32_t GetFieldCount() const
    {
        return _fieldCount;
    }

    [[nodiscard]] Field *Fetch() const;

    Field const &operator[](std::size_t index) const;

private:
    void CleanUp();
    bool FetchNextRow();

private:
    std::vector<QueryResultFieldMetadata> _fieldMetadata;
    std::vector<Field>                    _rows;
    uint64_t                              _rowCount;
    uint64_t                              _rowPosition;
    uint32_t                              _fieldCount;

    MySqlBind   *_pBind;
    MySqlStmt   *_pStmt;
    MySqlResult *_pMetadataResult; ///< Field metadata, returned by mysql_stmt_result_metadata
};
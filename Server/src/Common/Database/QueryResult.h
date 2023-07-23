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

class ResultSet
{
public:
    ResultSet(MySqlResult *result, MySqlField *fields, uint64_t rowCount, uint32_t fieldCount);
    ~ResultSet();

    bool     NextRow();
    uint64_t GetRowCount() const
    {
        return _rowCount;
    }
    uint32_t GetFieldCount() const
    {
        return _fieldCount;
    }

    // Field *Fetch() const
    // {
    //     return _currentRow;
    // }
    // Field const &operator[](std::size_t index) const;

protected:
    // std::vector<QueryResultFieldMetadata> _fieldMetadata;
    // uint64                                _rowCount;
    // Field                                *_currentRow;
    // uint32                                _fieldCount;

private:
    void         CleanUp();
    MySqlResult *_result;
    MySqlField  *_fields;

    ResultSet(ResultSet const &right)            = delete;
    ResultSet &operator=(ResultSet const &right) = delete;
};

class PreparedResultSet
{
public:
    PreparedResultSet(MySQLStmt *stmt, MySqlResult *result, uint64 rowCount, uint32 fieldCount);
    ~PreparedResultSet();

    bool   NextRow();
    uint64 GetRowCount() const
    {
        return m_rowCount;
    }
    uint32 GetFieldCount() const
    {
        return m_fieldCount;
    }

    Field       *Fetch() const;
    Field const &operator[](std::size_t index) const;

protected:
    std::vector<QueryResultFieldMetadata> m_fieldMetadata;
    std::vector<Field>                    m_rows;
    uint64                                m_rowCount;
    uint64                                m_rowPosition;
    uint32                                m_fieldCount;

private:
    MySQLBind   *m_rBind;
    MySQLStmt   *m_stmt;
    MySqlResult *m_metadataResult; ///< Field metadata, returned by mysql_stmt_result_metadata

    void CleanUp();
    bool _NextRow();

    PreparedResultSet(PreparedResultSet const &right)            = delete;
    PreparedResultSet &operator=(PreparedResultSet const &right) = delete;
};
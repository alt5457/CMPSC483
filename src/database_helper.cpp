#include "database_helper.h"
using json = nlohmann::json;

void DatabaseHelper::createAssignmentsTable() {
    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS ASSIGNMENTS ("
          "CAMPUSID TEXT PRIMARY KEY NOT NULL,"
          "MAJOR TEXT NOT NULL,"
          "PROJECTID TEXT NOT NULL,"
          "COMMENT TEXT,"
          "LASTNAME TEXT,"
          "FIRSTNAME TEXT NOT NULL,"
          "NDA INT,"
          "IP INT,"
          "ONCAMPUS INT);";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &db_err);
    if (rc != SQLITE_OK) {
        std::cout << "Fail to create table ASSIGNMENTS with sqlite error " << db_err << std::endl;
        sqlite3_free(db_err);
        throw "Fail to create table ASSIGNMENTS";
    }
    sqlite3_close(db);
}

void DatabaseHelper::createStudentsTable() {
    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS STUDENTS ("
          "CAMPUSID TEXT NOT NULL,"
          "PROJECTID TEXT NOT NULL,"
          "MAJOR TEXT NOT NULL,"
          "TIMEA TEXT,"
          "TIMEB TEXT,"
          "TIMEC TEXT,"
          "PREFERRED INT,"
          "COMMENT TEXT,"
          "NDA TEXT,"
          "IP TEXT,"
          "LASTNAME INT,"
          "FIRSTNAME INT,"
          "ONCAMPUS INT,"
          "PRIMARY KEY (CAMPUSID, PROJECTID) );";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &db_err);
    if (rc != SQLITE_OK) {
        std::cout << "Fail to create table STUDENTS with sqlite error " << db_err << std::endl;
        sqlite3_free(db_err);
        throw "Fail to create table STUDENTS";
    }
    sqlite3_close(db);
}

void DatabaseHelper::addStudents(const char *csv_filepath) {
    auto students = rapidcsv::Document(csv_filepath, rapidcsv::LabelParams(0, -1));
    size_t count = students.GetRowCount();

    auto campus_ids  = students.GetColumn<std::string>("campus_id");
    auto project_ids = students.GetColumn<std::string>("ProjectID");
    auto majors      = students.GetColumn<std::string>("Major");
    auto timeAs      = students.GetColumn<std::string>("TimeA");
    auto timeBs      = students.GetColumn<std::string>("TimeB");
    auto timeCs      = students.GetColumn<std::string>("TimeC");
    auto preferreds  = students.GetColumn<std::string>("Preferred"); 
    auto comments    = students.GetColumn<std::string>("Comments");
    auto ndas        = students.GetColumn<std::string>("Student_NDA");
    auto ips         = students.GetColumn<std::string>("Student_IP");
    auto last_names  = students.GetColumn<std::string>("last_name");
    auto first_names = students.GetColumn<std::string>("first_name");    
    auto on_campus   = students.GetColumn<std::string>("OnCampus");

    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    for (size_t i = 0; i < count; i++) {
        std::string sql = "INSERT INTO STUDENTS (CAMPUSID, PROJECTID, MAJOR, TIMEA, TIMEB, TIMEC, PREFERRED, COMMENT, NDA, IP, LASTNAME, FIRSTNAME, ONCAMPUS) ";
        sql += "VALUES (";
        sql += "'" + campus_ids[i] + "', ";
        sql += "'" + project_ids[i] + "', ";
        sql += "'" + majors[i] + "', ";
        sql += "'" + timeAs[i] + "', ";
        sql += "'" + timeBs[i] + "', ";
        sql += "'" + timeCs[i] + "', ";
        sql += "'" + preferreds[i] + "', ";
        sql += "'" + comments[i] + "', ";
        sql += "'" + ndas[i] + "', ";
        sql += "'" + ips[i] + "', ";
        sql += "'" + last_names[i] + "', ";
        sql += "'" + first_names[i] + "', ";
        sql += "'" + on_campus[i] + "'";
        sql += ");";

        std::cout << "Executing SQL query " << sql << std::endl;
        rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &db_err);
        if (rc != SQLITE_OK) {
            std::cout << "Fail to add students with sqlite error " << db_err << std::endl;
            sqlite3_free(db_err);
            throw "Fail to add students";
        } 
    }

    sqlite3_close(db);
}

void DatabaseHelper::addAssignments(std::unordered_map<std::string, StudentData> &studentsMap) {
    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    for (const auto & item : studentsMap) {
        std::string campus_id = item.first;
        auto studentData = item.second;
        std::string major = majorTypesToString(studentData.m_major);
        std::string project_id = studentData.m_project_id;
        std::string comment = studentData.m_comment;
        std::string lastname = studentData.m_last_name;
        std::string firstname = studentData.m_first_name;
        std::string nda = studentData.m_nda? "1" : "0";
        std::string ip = studentData.m_ip? "1" : "0";
        std::string oncampus = studentData.m_on_campus? "1" : "0";

        std::string sql = "INSERT INTO ASSIGNMENTS (CAMPUSID, MAJOR, PROJECTID, COMMENT, LASTNAME, FIRSTNAME, NDA, IP, ONCAMPUS) ";
        sql += "VALUES ('" + campus_id + "', '" + major + "', '" + project_id + "', '" + comment + "', '" + lastname + "', '" + firstname + "', " + nda + ", " + ip + ", " + oncampus + ");";

        std::cout << "Executing SQL query " << sql << std::endl;
        rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &db_err);
        if (rc != SQLITE_OK) {
            std::cout << "Fail to add assignments with sqlite error " << db_err << std::endl;
            sqlite3_free(db_err);
            throw "Fail to add assignments";
        }
    }

    sqlite3_close(db);
}

static int jsonCallback(void *data, int c_num, char **c_vals, char **c_names) {
    json *json_data = (json *)data;
    for (int i = 1; i < c_num; i++) {
        std::cout << "Adding to JSON key = " << c_vals[0] << " with " << c_names[i] << " = " << c_vals[i] << std::endl;
        (*json_data)[c_vals[0]][c_names[i]] = c_vals[i];
    }
    return 0;
}

json DatabaseHelper::getStudentsUnderProject(const char *projectID) {
    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    std::string sql = "SELECT ASSIGNMENTS.CAMPUSID, ASSIGNMENTS.FIRSTNAME, ASSIGNMENTS.LASTNAME, ASSIGNMENTS.MAJOR, ASSIGNMENTS.COMMENT, STUDENTS.PREFERRED "
                      "FROM ASSIGNMENTS, STUDENTS "
                      "WHERE ASSIGNMENTS.PROJECTID = '";
    sql += projectID;
    sql += "' AND ASSIGNMENTS.CAMPUSID = STUDENTS.CAMPUSID;";


    std::cout << "Executing SQL query " << sql << std::endl;
    json data;
    rc = sqlite3_exec(db, sql.c_str(), jsonCallback, (void *)(&data), &db_err);
    if (rc != SQLITE_OK) {
        std::cout << "Fail to get students under project with sqlite error " << db_err << std::endl;
        sqlite3_free(db_err);
        throw "Fail to get students under project ";
    }

    sqlite3_close(db);
    return data;
}

json DatabaseHelper::getAssignments() {
    char *db_err;
    int rc = sqlite3_open(db_name, &db);
    if (rc) {
        throw "Can't open database";
    }

    std::string sql = "SELECT ASSIGNMENTS.CAMPUSID, ASSIGNMENTS.FIRSTNAME, ASSIGNMENTS.LASTNAME, ASSIGNMENTS.MAJOR, ASSIGNMENTS.COMMENT, STUDENTS.PREFERRED "
                      "FROM ASSIGNMENTS, STUDENTS "
                      "WHERE ASSIGNMENTS.CAMPUSID = STUDENTS.CAMPUSID;";

    std::cout << "Executing SQL query " << sql << std::endl;
    json data;
    rc = sqlite3_exec(db, sql.c_str(), jsonCallback, (void *)(&data), &db_err);
    if (rc != SQLITE_OK) {
        std::cout << "Fail to get assignments with sqlite error " << db_err << std::endl;
        sqlite3_free(db_err);
        throw "Fail to get assignments under project ";
    }

    sqlite3_close(db);
    return data;    
}
#include "csv_handler.h"
using json = nlohmann::json;

ProjectDataHandler::ProjectDataHandler(const char *csv_filepath) noexcept {
        parse(csv_filepath);
}
void ProjectDataHandler::parse(const char *csv_filepath) noexcept{
        auto projects = rapidcsv::Document(csv_filepath, rapidcsv::LabelParams(0, -1));
        
        size_t m_count = projects.GetRowCount();
        
        std::vector<std::string> project_ids    = projects.GetColumn<std::string>("ProjectID");
        std::vector<std::string> company_names  = projects.GetColumn<std::string>("CompanyName");
        std::vector<std::string> project_titles = projects.GetColumn<std::string>("ProjectTitle");
        
        std::vector<std::array<int, magic_enum::enum_count<StudentMajorTypes>()>> major_preferences;
        for (int i = 0; i < m_count; i++) {
                std::array<int, magic_enum::enum_count<StudentMajorTypes>()> preferences{};
                for (int j = 0; j < magic_enum::enum_count<StudentMajorTypes>(); j++)
                        preferences[j] = projects.GetCell<int>(std::string(magic_enum::enum_name((StudentMajorTypes)j)), i);
                major_preferences.push_back(preferences);
        }
        
        std::vector<std::string> course_times        = projects.GetColumn<std::string>("CourseTime");
        std::vector<std::string> course_names        = projects.GetColumn<std::string>("CourseName");
        std::vector<int> confidentialities           = projects.GetColumn<int>("Confidentiality");
        std::vector<int> ips                         = projects.GetColumn<int>("IP");
        std::vector<int> physical_prototypes         = projects.GetColumn<int>("PhysicalPrototype");
        
        projects.Clear();
        
        for (int i = 0; i < m_count; i++) {
                ProjectData data = {
                        .m_course_time        = course_times[i],
                        .m_course_name        = course_names[i],
                        .m_company_name       = company_names[i],
                        .m_project_title      = project_titles[i],
                        .m_confidentiality    = static_cast<bool>(confidentialities[i]),
                        .m_ip                 = static_cast<bool>(ips[i]),
                        .m_physical_prototype = static_cast<bool>(physical_prototypes[i]),
                };
                
                for (int major = 0; major < magic_enum::enum_count<StudentMajorTypes>(); major++){
                        int rank = major_preferences[i][major];
                        switch (rank) {
                                case 0: continue;
                                case 1: data.m_first_preference = static_cast<StudentMajorTypes>(major); break;
                                case 2: data.m_second_preference = static_cast<StudentMajorTypes>(major); break;
                                case 3: data.m_third_preferences.push_back(static_cast<StudentMajorTypes>(major)); break;
                                default: ERROR("INVALID RANK: RANK SHOULD BE 3 OR BELOW, (%s, %s)", std::string(data.m_project_title).c_str(), std::string(magic_enum::enum_name(static_cast<StudentMajorTypes>(major))).c_str()); break;
                        }
                }
                
                m_projects_map[project_ids[i]] = data;

                // fill in projects json
                size_t pos = course_times[i].find("-");
                m_projects_json[project_ids[i]]["courseTime"] = course_times[i].substr(4, pos-4);
                m_projects_json[project_ids[i]]["courseName"] = course_names[i];
                m_projects_json[project_ids[i]]["companyName"] = company_names[i];
                m_projects_json[project_ids[i]]["projectTitle"] = project_titles[i];
                m_projects_json[project_ids[i]]["confidentiality"] = confidentialities[i]? "Yes" : "No";
                m_projects_json[project_ids[i]]["ip"] = ips[i]? "Yes" : "No";
                m_projects_json[project_ids[i]]["physicalPrototype"] = physical_prototypes[i]? "Yes" : "No";
                m_projects_json[project_ids[i]]["firstPreference"] = majorTypesToString(data.m_first_preference);
                m_projects_json[project_ids[i]]["secondPreference"] = majorTypesToString(data.m_second_preference);
                std::string thirdPreference = "";
                for (size_t i = 0; i < data.m_third_preferences.size(); i++) {
                        if (i == 0) {
                                thirdPreference += majorTypesToString(data.m_third_preferences[0]);
                        } else {
                                thirdPreference += ", " + majorTypesToString(data.m_third_preferences[i]);
                        }
                        
                }
                m_projects_json[project_ids[i]]["thirdPreference"] = thirdPreference;
        }
}
std::unordered_map<std::string, ProjectData> &ProjectDataHandler::projectsMap() {
        return m_projects_map;
}

json &ProjectDataHandler::projectsJson() {
        return m_projects_json;
}

void StudentDataHandler::parse(const char *csv_filepath) noexcept {
        auto students = rapidcsv::Document(csv_filepath, rapidcsv::LabelParams(0, -1));
        
        std::vector<StudentMajorTypes> majors(students.GetRowCount());
        
        for (int i = 0; const auto& major : students.GetColumn<std::string>("Major"))
                majors[i] = magic_enum::enum_cast<StudentMajorTypes>(major).value();
        
        
        auto project_ids = students.GetColumn<std::string>("ProjectID");
        auto comments    = students.GetColumn<std::string>("Comments");
        auto campus_ids  = students.GetColumn<std::string>("campus_id");
        auto last_names  = students.GetColumn<std::string>("last_name");
        auto first_names = students.GetColumn<std::string>("first_name");
        auto ndas        = students.GetColumn<int>("Student_NDA");
        auto ips         = students.GetColumn<int>("Student_IP");
        auto on_campus   = students.GetColumn<std::string>("OnCampus");
        std::vector<bool> on_campus_bools;
        on_campus_bools.reserve(on_campus.size());
        
        for (const auto& str : on_campus){
                if (str == "Yes")
                        on_campus_bools.push_back(true);
                else
                        on_campus_bools.push_back(false);
        }
        
        std::unordered_map<std::string, std::vector<std::string>> project_students_map;
        for (int i = 0; i < students.GetRowCount(); i++){
                StudentData data = {
                        .m_major      = majors[i],
                        .m_comment    = comments[i],
                        .m_project_id = project_ids[i],
                        .m_last_name  = last_names[i],
                        .m_first_name = first_names[i],
                        .m_campus_id  = campus_ids[i],
                        .m_nda        = static_cast<bool>(ndas[i]),
                        .m_ip         = static_cast<bool>(ips[i]),
                        .m_on_campus  = on_campus_bools[i],
                };
                
                m_students_map[campus_ids[i]] = data;
                project_students_map[project_ids[i]].push_back(campus_ids[i]);
        }
        m_project_students_map = project_students_map;
}
void StudentDataHandler::parse(const std::string &body) noexcept {
        std::stringstream sstream(body);
        rapidcsv::Document students(sstream, rapidcsv::LabelParams(0, -1));
        
        std::vector<StudentMajorTypes> majors(students.GetRowCount());
        
        for (int i = 0; const auto& major : students.GetColumn<std::string>("Major"))
                majors[i] = magic_enum::enum_cast<StudentMajorTypes>(major).value();
        
        auto project_ids = students.GetColumn<std::string>("ProjectID");
        auto comments    = students.GetColumn<std::string>("Comments");
        auto campus_ids  = students.GetColumn<std::string>("campus_id");
        auto last_names  = students.GetColumn<std::string>("last_name");
        auto first_names = students.GetColumn<std::string>("first_name");
        auto ndas        = students.GetColumn<int>("Student_NDA");
        auto ips         = students.GetColumn<int>("Student_IP");
        auto on_campus   = students.GetColumn<std::string>("OnCampus");
        std::vector<bool> on_campus_bools;
        on_campus_bools.reserve(on_campus.size());
        
        for (const auto &str : on_campus){
                if (str == "Yes")
                        on_campus_bools.push_back(true);
                else
                        on_campus_bools.push_back(false);
        }
        
        std::unordered_map<std::string, std::vector<std::string>> project_students_map;
        for (int i = 0; i < students.GetRowCount(); i++){
                StudentData data = {
                        .m_major      = majors[i],
                        .m_comment    = comments[i],
                        .m_project_id = project_ids[i],
                        .m_last_name  = last_names[i],
                        .m_first_name = first_names[i],
                        .m_campus_id  = campus_ids[i],
                        .m_nda        = static_cast<bool>(ndas[i]),
                        .m_ip         = static_cast<bool>(ips[i]),
                        .m_on_campus  = on_campus_bools[i],
                };
                
                m_students_map[campus_ids[i]] = data;
                project_students_map[project_ids[i]].push_back(campus_ids[i]);
        }
        m_project_students_map = project_students_map;
}
std::unordered_map<std::string, StudentData>& StudentDataHandler::studentsMap() {
        return m_students_map;
}
std::unordered_map<std::string, std::vector<std::string>>& StudentDataHandler::projectStudentsMap() {
        return m_project_students_map;
}

InstructorDataHandler::InstructorDataHandler(const char *csv_filepath) noexcept {
        parse(csv_filepath);
}
void InstructorDataHandler::parse(const char *csv_filepath) noexcept {
        auto instructors = rapidcsv::Document(csv_filepath, rapidcsv::LabelParams(0, -1));
        
        std::vector<std::string> instructor_names = instructors.GetColumn<std::string>("InstructorName");
        std::vector<std::string> project_ids      = instructors.GetColumn<std::string>("ProjectID");
        std::unordered_map<std::string, std::vector<std::string>> instructor_projects_map;
        
        
        for (int i = 0; i < instructors.GetRowCount(); i++) {
                instructor_projects_map[instructor_names[i]].push_back(project_ids[i]);

                // fill in projects instructor map json
                m_projects_instructor_map_json[project_ids[i]] = instructor_names[i];
        }         
        m_instructor_projects_map = instructor_projects_map;
        
}
std::unordered_map<std::string, std::vector<std::string>> &InstructorDataHandler::instructorProjectsMap() { return m_instructor_projects_map; }

json &InstructorDataHandler::projectsInstructorMapJson() {
        return m_projects_instructor_map_json;
}

CsvHandler::CsvHandler(ProjectDataHandler &project_data, StudentDataHandler &student_data, InstructorDataHandler &instructor_data)
: m_project_data(project_data), m_student_data(student_data), m_instructor_data(instructor_data){

}
std::string CsvHandler::pretty_format(){
        std::string ret;
        for (const auto& instructor_projects_pair : m_instructor_data.instructorProjectsMap()){
                ret += instructor_projects_pair.first + ":\n";
                for (const auto& project_id : instructor_projects_pair.second){
                        ret += "\t" + project_id + ":\n";
                        for (const auto& student_id : (m_student_data.projectStudentsMap())[project_id])
                                ret += "\t\t" + m_student_data.studentsMap()[student_id].m_first_name + " " + m_student_data.studentsMap()[student_id].m_last_name + "(" + student_id + ")" + "\n";
                }
        }
        return ret;
}
std::string CsvHandler::json_format() {
        std::string ret;
        return std::string();
}
std::string CsvHandler::simple_format() {
        std::string ret;
        ret += "Instructor,Project,Students\n";
        for (const auto& instructor_projects_pair : m_instructor_data.instructorProjectsMap()){
                for (const auto& project_id : instructor_projects_pair.second){
                        ret += instructor_projects_pair.first + "," + m_project_data.projectsMap()[project_id].m_project_title;
                        ret += ",{";
                        for (const auto& student_id : (m_student_data.projectStudentsMap())[project_id]){
                                // std::cout << student_id << "\n";
                                ret += m_student_data.studentsMap()[student_id].m_first_name + " " + m_student_data.studentsMap()[student_id].m_last_name + "(" + student_id + ")" + "     ";
                        }
                        // std::cout << "\n";
                        ret += "}\n";
                }
        }
        return ret;
}

ProjectDataHandler &CsvHandler::projectData()       { return m_project_data;    }
StudentDataHandler &CsvHandler::studentData()       { return m_student_data;    }
InstructorDataHandler &CsvHandler::instructorData() { return m_instructor_data; }


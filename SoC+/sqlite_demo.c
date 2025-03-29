#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

int execute_sql(sqlite3 *db, const char *sql) {
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    return SQLITE_OK;
}


// ═══════════════════════  List of Student Functions [Start] ═══════════════════════
void add_student(sqlite3 *db) {
    char first_name[50], last_name[50];
    printf("Enter first name: ");
    scanf("%49s", first_name);
    printf("Enter last name: ");
    scanf("%49s", last_name);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT student_id FROM Students WHERE first_name = ? AND last_name = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, first_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, last_name, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("This student already exists in the database.\n");
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Students (first_name, last_name, gwa) VALUES ('%s', '%s', NULL);", first_name, last_name);
    execute_sql(db, sql);
    printf("Student added successfully.\n");
}

void view_students(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM Students;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("Student Records:\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d | Name: %s %s | GWA: %s\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_type(stmt, 3) != SQLITE_NULL ? sqlite3_column_text(stmt, 3) : (const unsigned char *)"N/A");
        }
        sqlite3_finalize(stmt);
    }
}

void delete_student(sqlite3 *db) {
    int student_id;
    printf("Enter Student ID to delete: ");
    scanf("%d", &student_id);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT student_id FROM Students WHERE student_id = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            printf("Student ID %d does not exist.\n", student_id);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[128];
    snprintf(sql, sizeof(sql), "PRAGMA foreign_keys = ON; DELETE FROM Students WHERE student_id = %d;", student_id);
    execute_sql(db, sql);
    printf("Student deleted successfully.\n");
}

void view_student_grades(sqlite3 *db) {
    int student_id;
    printf("Enter Student ID to view grades: ");
    scanf("%d", &student_id);

    sqlite3_stmt *stmt;
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT g.course_code, c.course_name, c.course_units, g.units_attained "
             "FROM Grades g "
             "JOIN Courses c ON g.course_code = c.course_code "
             "WHERE g.student_id = %d;", student_id);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("\nGrades for Student ID %d:\n", student_id);
        printf("Course Code | Course Name | Units Attained | Grade\n");
        printf("----------------------------------------\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("%s | %s | %d | %s\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_int(stmt, 2),
                   sqlite3_column_type(stmt, 3) != SQLITE_NULL ? sqlite3_column_text(stmt, 3) : (const unsigned char *)"N/A");
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error retrieving grades.\n");
    }
}


void add_student_grade(sqlite3 *db) {
    int student_id;
    char course_code[20];
    double grade;

    printf("Enter Student ID: ");
    scanf("%d", &student_id);
    printf("Enter Course Code: ");
    scanf("%19s", course_code);
    getchar();
    printf("Enter Grade: ");
    scanf("%lf", &grade);
    printf("");

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT student_id, course_code FROM Grades WHERE student_id = ? AND course_code = ?;";

    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        sqlite3_bind_text(stmt, 2, course_code, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("This student already has a grade for this course. Updating the grade...\n");
            const char *sql_update = "UPDATE Grades SET units_attained = ? WHERE student_id = ? AND course_code = ?;";
            sqlite3_stmt *update_stmt;
            if (sqlite3_prepare_v2(db, sql_update, -1, &update_stmt, 0) == SQLITE_OK) {
                sqlite3_bind_double(update_stmt, 1, grade);
                sqlite3_bind_int(update_stmt, 2, student_id);
                sqlite3_bind_text(update_stmt, 3, course_code, -1, SQLITE_STATIC);
                if (sqlite3_step(update_stmt) == SQLITE_DONE) {
                    printf("Grade updated successfully.\n");
                }
                sqlite3_finalize(update_stmt);
            }
        } else {
            printf("Assigning grade to student for the course...\n");
            const char *sql_insert = "INSERT INTO Grades (student_id, course_code, units_attained) VALUES (?, ?, ?);";
            sqlite3_stmt *insert_stmt;
            if (sqlite3_prepare_v2(db, sql_insert, -1, &insert_stmt, 0) == SQLITE_OK) {
                sqlite3_bind_int(insert_stmt, 1, student_id);
                sqlite3_bind_text(insert_stmt, 2, course_code, -1, SQLITE_STATIC);
                sqlite3_bind_double(insert_stmt, 3, grade);
                if (sqlite3_step(insert_stmt) == SQLITE_DONE) {
                    printf("Grade assigned successfully.\n");
                }
                sqlite3_finalize(insert_stmt);
            }
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error checking existing grade.\n");
    }
   
}

void compute_gwa(sqlite3 *db) {
    int student_id;
    printf("Enter Student ID: ");
    scanf("%d", &student_id);

    sqlite3_stmt *stmt;
    // const char *sql = "SELECT g.grade, g.units_attained FROM Grades g WHERE g.student_id = ?;";
    const char *sql = "SELECT g.units_attained, c.course_units FROM Grades g JOIN courses c USING (course_code) WHERE g.student_id = ?;";
	

    double total_units = 0;
    double weighted_sum = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, student_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                double grade = sqlite3_column_double(stmt, 0);
                int units = sqlite3_column_int(stmt, 1);
                weighted_sum += grade * units;
                total_units += units;
            }
        }
        sqlite3_finalize(stmt);

        if (total_units > 0) {
            double gwa = weighted_sum / total_units;
            printf("GWA for Student ID %d: %.2f\n", student_id, gwa);

            char update_sql[128];
            snprintf(update_sql, sizeof(update_sql), "UPDATE Students SET gwa = %.2f WHERE student_id = %d;", gwa, student_id);
            execute_sql(db, update_sql);
        } else {
            printf("No grades found for this student.\n");
        }
    } else {
        printf("Error retrieving grades.\n");
    }
}

// ═══════════════════════ Student Functions [End] ═══════════════════════════════════════
//
// ═══════════════════════ Nested Switch Case for students [Start] ═══════════════════════
void student_menu(sqlite3 *db) {
    int choice;
    do {
        printf("\nStudent Options:\n");
        printf("1. Add Student\n");
        printf("2. View Students\n");
        printf("3. Delete Student\n");
        printf("4. Add/Update Student Grades\n");
        printf("5. View Student Grades\n");
        printf("6. Compute GWA\n");
        printf("7. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_student(db); break;
            case 2: view_students(db); break;
            case 3: delete_student(db); break;
            case 4: add_student_grade(db); break;
            case 5: view_student_grades(db); break;
            case 6: compute_gwa(db); break;
            case 7: printf("Returning to Main Menu.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 7);
}
// ═══════════════════════ Nested Switch Case for students [End] ═══════════════════════
//
// ═══════════════════════ Professor Functions [Start] ═════════════════════════════════
void add_professor(sqlite3 *db) {
    char first_name[50], last_name[50], masters_degree[100];
    printf("Enter first name: ");
    scanf("%49s", first_name);
    printf("Enter last name: ");
    scanf("%49s", last_name);
    printf("Enter master's degree: ");
    scanf("%99s", masters_degree);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT professor_id FROM Professors WHERE first_name = ? AND last_name = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, first_name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, last_name, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Professor %s %s already exists.\n", first_name, last_name);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[256];
    snprintf(sql, sizeof(sql),
             "INSERT INTO Professors (first_name, last_name, masters_degree) VALUES ('%s', '%s', '%s');",
             first_name, last_name, masters_degree);

    execute_sql(db, sql);
    printf("Professor added successfully.\n");
}

void view_professors(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM Professors;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("Professor Records:\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d | Name: %s %s | Master's Degree: %s\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2),
                   sqlite3_column_type(stmt, 3) != SQLITE_NULL ? sqlite3_column_text(stmt, 3) : (const unsigned char *)"N/A");
        }
        sqlite3_finalize(stmt);
    }
}

void delete_professor(sqlite3 *db) {
    int professor_id;
    printf("Enter Professor ID to delete: ");
    scanf("%d", &professor_id);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT professor_id FROM Professors WHERE professor_id = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, professor_id);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            printf("Professor ID %d does not exist.\n", professor_id);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[128];
    snprintf(sql, sizeof(sql), "PRAGMA foreign_keys = ON; DELETE FROM Professors WHERE professor_id = %d;", professor_id);
    execute_sql(db, sql);
    printf("Professor deleted successfully.\n");
}

void assign_professor(sqlite3 *db) {
    int professor_id;
    char course_code[20];

    printf("Enter Course Code: ");
    scanf("%19s", course_code);
    getchar();
    printf("Enter Professor ID: ");
    scanf("%d", &professor_id);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT professor_id FROM Professors WHERE professor_id = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, professor_id);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            printf("Professor ID %d does not exist.\n", professor_id);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE Courses SET professor_id = %d WHERE course_code = '%s';", professor_id, course_code);

    execute_sql(db, sql);
    printf("Professor assigned to course successfully.\n");
}
// ═══════════════════════ Professor Functions [End] ═══════════════════════════════════
//
// ═══════════════════════ Nested Switch Case for Professors [Start] ═══════════════════
void professor_menu(sqlite3 *db) {
    int choice;
    do {
        printf("\nProfessor Options:\n");
        printf("1. Add Professor\n");
        printf("2. View Professors\n");
        printf("3. Delete Professor\n");
        printf("4. Assign Professor to Course\n");
        printf("5. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_professor(db); break;
            case 2: view_professors(db); break;
            case 3: delete_professor(db); break;
            case 4: assign_professor(db); break;
            case 5: printf("Returning to Main Menu.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

// ═══════════════════════ Nested Switch Case for Professors [End] ═════════════════════
//
// ═══════════════════════ Course Functions [Start] ════════════════════════════════════
void add_course(sqlite3 *db) {
    char course_code[20], course_name[100];
    int no_units;

    printf("Enter course code: ");
    scanf("%19s", course_code);
    printf("Enter course name: ");
    scanf(" %99[^\n]", course_name); 
    printf("Enter number of units: ");
    scanf("%d", &no_units);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT course_code FROM Courses WHERE course_code = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, course_code, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Course code %s already exists.\n", course_code);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Courses (course_code, course_name, professor_id, course_units) VALUES ('%s', '%s', NULL, %d);", course_code, course_name, no_units);
    execute_sql(db, sql);
    printf("Course added successfully.\n");
}

void view_courses(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM Courses;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("Course Records:\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Code: %s | Name: %s | Units: %d | Professor ID: %s\n",
                   sqlite3_column_text(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_int(stmt, 3),
                   sqlite3_column_type(stmt, 2) != SQLITE_NULL ? sqlite3_column_text(stmt, 2) : (const unsigned char *)"None");
        }
        sqlite3_finalize(stmt);
    }
}

void delete_course(sqlite3 *db) {
    char course_code[20];
    printf("Enter course code to delete: ");
    scanf("%19s", course_code);

    sqlite3_stmt *stmt;
    const char *sql_check = "SELECT course_code FROM Courses WHERE course_code = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, course_code, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            printf("Course code %s does not exist.\n", course_code);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }

    char sql[128];
    snprintf(sql, sizeof(sql), "PRAGMA foreign_keys = ON; DELETE FROM Courses WHERE course_code = '%s';", course_code);
    execute_sql(db, sql);
    printf("Course deleted successfully.\n");
}
void view_students_in_course(sqlite3 *db) {
    char course_code[20];
    printf("Enter course code: ");
    scanf("%19s", course_code);

    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT s.first_name, s.last_name, g.units_attained "
        "FROM Grades g "
        "JOIN Students s ON g.student_id = s.student_id "
        "WHERE g.course_code = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, course_code, -1, SQLITE_STATIC);

        printf("\nStudents enrolled in %s:\n", course_code);
        printf("First Name | Last Name | Units Attained\n");
        printf("---------------------------------------\n");

        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = 1;

            // Check if units_attained is NULL
            if (sqlite3_column_type(stmt, 2) == SQLITE_NULL) {
                printf("%s | %s | N/A\n",
                       sqlite3_column_text(stmt, 0),
                       sqlite3_column_text(stmt, 1));
            } else {
                printf("%s | %s | %.2f\n",
                       sqlite3_column_text(stmt, 0),
                       sqlite3_column_text(stmt, 1),
                       sqlite3_column_double(stmt, 2));  // Correct float handling
            }
        }

        if (!found) {
            printf("No students found for this course.\n");
        }

        sqlite3_finalize(stmt);
    } else {
        printf("Error fetching students: %s\n", sqlite3_errmsg(db));
    }
}

// ═══════════════════════ Course Functions [End] ═════════════════════════════════════
//
// ═══════════════════════ Nested Switch Case for Course [Start] ═════════════════════════
void course_menu(sqlite3 *db) {
    int choice;
    do {
        printf("\nCourse Options:\n");
        printf("1. Add Course\n");
        printf("2. View Courses\n");
        printf("3. Delete Course\n");
        printf("4. View Students Enrolled in a Course\n");
        printf("5. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_course(db); break;
            case 2: view_courses(db); break;
            case 3: delete_course(db); break;
            case 4: view_students_in_course(db); break;
            case 5: printf("Returning to Main Menu.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

// ═══════════════════════ Nested Switch Case for Course [End] ═════════════════════════
//
// ═══════════════════════ Main Switch Case [Start] ════════════════════════════════════
void main_menu(sqlite3 *db) {
    int choice;
    do {
        printf("\nAdmin Menu:\n");
        printf("1. Student Options\n");
        printf("2. Professor Options\n");
        printf("3. Course Options\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: student_menu(db); break;
            case 2: professor_menu(db); break;
            case 3: course_menu(db); break;
            case 4: printf("Exiting program.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 4);
}
// ═══════════════════════ Main Switch Case [End] ════════════════════════════════════════
//
//════════════════════════════════════════════════════════════════════════════════════════
int main() {
    sqlite3 *db;
    int rc = sqlite3_open("school.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    main_menu(db);

    sqlite3_close(db);
    return 0;
}

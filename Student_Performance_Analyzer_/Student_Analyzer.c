#include<stdio.h>
#include<string.h>
#define SUBJECT_COUNT 3

const int MIN_STUDENTS = 1;
const int MAX_STUDENTS = 100;
const int MIN_MARKS = 0;   
const int MAX_MARKS = 100;

// structure to store student details
struct Student_details
{
    char name[50]; //to store student'student name
    unsigned int rollNumber; //to store roll number 
    unsigned short marks[SUBJECT_COUNT]; //to store marks
    unsigned short totalMarks;  //to store totalMarks marks
    float average;  //to store average of marks
    char  grade; //to store assigned grade
};


//Calculate totalMarks marks using arithmetic operations
int total_marks(const struct Student_details *student) {
    int totalMarks = 0;
    for (int i = 0; i < SUBJECT_COUNT; i++) {
        totalMarks += student->marks[i];
    }
    return totalMarks;
}

//calculate  average marks
float average_marks(const struct Student_details *student){
        return student->totalMarks / (float)SUBJECT_COUNT;
}

//function for grade
char student_grade(const struct Student_details *student){
        float average=student->average;
        if (average >= 85)
        return 'A';
    else if (average >= 70)
        return 'B';
    else if (average >= 50)
        return 'C';
    else if (average >= 35)
        return 'D';
    else
        return 'F';

}


// Function to print stars based on grade
void performance_stars(char grade){
    switch(grade){
        case 'A': printf("Performance: *****\n"); break;
        case 'B': printf("Performance: ****\n"); break;
        case 'C': printf("Performance: ***\n"); break;
        case 'D': printf("Performance: **\n"); break;
        default:  printf("Performance: \n"); 
    }
}

// recurcive function for print roll number
void print_roll_numbers(const struct Student_details student[], int n, int index){
    if(index == n) return; // base case
    printf("%d ", student[index].rollNumber);
    print_roll_numbers(student, n, index+1); 
}


int main(){

    int number_students;//to store the number of students
    scanf("%d",&number_students);
    getchar();
    if (number_students>MAX_STUDENTS || number_students<MIN_STUDENTS)
    {
        printf("Number of Students Must be Between 1 - 100 \n");
        return 1;
    }

    // Create an array of structures
    struct Student_details students[MAX_STUDENTS];
    
    
    //taking the input of each students here 
    char buffer [100];
    for(int i=0;i<number_students;i++){
    while(1){
    
        fgets(buffer, sizeof(buffer),stdin);

        unsigned int roll_number, marks_1, marks_2, marks_3;
        char name_[50];

        int fields = sscanf(buffer, "%u %s %d %d %d", &roll_number, name_, &marks_1, &marks_2, &marks_3);

        if(fields == 5){
             if ((marks_1 < MIN_MARKS || marks_1 > MAX_MARKS) ||
                (marks_2 < MIN_MARKS || marks_2 > MAX_MARKS) ||
                (marks_3 < MIN_MARKS || marks_3 > MAX_MARKS)) {
                printf("Error: Marks must be between %d and %d. Please re-enter this student's data.\n",
                       MIN_MARKS, MAX_MARKS);
                continue; // ask again for input
            }
            students[i].rollNumber = roll_number;
                strcpy(students[i].name, name_);
                students[i].marks[0] = marks_1;
                students[i].marks[1] = marks_2;
                students[i].marks[2] = marks_3;
                break;
            } 
            else {
                printf("Invalid input format! Please try again.\n");
            }   
        }
        

       
        
     // call function
        students[i].totalMarks = total_marks(&students[i]);
        students[i].average = average_marks(&students[i]);
        students[i].grade = student_grade(&students[i]);

    }

     //Printing the totalMarks marks  of each students
    for(int i=0;i<number_students;i++){
        printf("Roll: %d\n",students[i].rollNumber);
        printf("Name: %s\n",students[i].name);
        printf("totalMarks: %d\n",students[i].totalMarks);
        printf("Average: %.2f\n",students[i].average);
        printf("Grade: %c\n",students[i].grade);
        
        
        if(students[i].average < 35){
        continue; // skip printing performance stars
    }
    performance_stars(students[i].grade);
    }
    printf("List of Roll Numbers (via recursion):" );
    print_roll_numbers(students, number_students, 0);
     return 0;
    
}


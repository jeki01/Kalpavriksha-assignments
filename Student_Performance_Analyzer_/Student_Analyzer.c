#include<stdio.h>

// structure to store student details
struct Student_details
{
    char name[50]; //to store student's name
    int RollNumber; //to store roll number 
    int marks[3]; //to store more of 3 subjects
    float total;  //to store total marks
    float average;  //to store average of marks
     char  grade; //to store assigned grade
};


//Calculate total marks using arithmetic operations
float total_marks(struct Student_details s){
    float total=0;
    for(int i=0;i<3;i++){
        total +=s.marks[i];
    }
    return  total;
}

//calculate  average marks
float average_marks(struct Student_details s){
        return s.total/3;
}

//function for grade
char student_grade(struct Student_details s){
        char grade;
        float average=s.average;
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
        default:  printf("Performance: \n"); // F or fail
    }
}

// recurcive function for print roll number
void print_roll_numbers(struct Student_details s[], int n, int index){
    if(index == n) return; // base case
    printf("%d ", s[index].RollNumber);
    print_roll_numbers(s, n, index+1); 
}


int main(){

    int number_students;//to store the number of students
    scanf("%d ",&number_students);

    // Create an array of structures
    struct Student_details students[number_students];

    //taking the input of each students here 
    for(int i=0;i<number_students;i++){
        scanf("%d%s%d%d%d",&students[i].RollNumber
            ,students[i].name,
            &students[i].marks[0],
            &students[i].marks[1],
            &students[i].marks[2]);
        
     // call function
        students[i].total = total_marks(students[i]);
        students[i].average = average_marks(students[i]);
        students[i].grade = student_grade(students[i]);

    }

     //Printing the total marks  of each students
    for(int i=0;i<number_students;i++){
        printf("Roll: %d\n",students[i].RollNumber);
        printf("Name: %s\n",students[i].name);
        printf("Total: %f\n",students[i].total);
        printf("Average: %f\n",students[i].average);
        printf("Grade: %c\n",students[i].grade);
        performance_stars(students[i].grade);
        
    }
    printf("List of Roll Numbers (via recursion):" );
    print_roll_numbers(students, number_students, 0);
     return 0;
    
}

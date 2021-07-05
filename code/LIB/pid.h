#ifndef _PID_H_
#define _PID_H_

typedef struct
{
    double Target;
    double Err;
    double P;
    double I;
    double D;
    double Output;
}pid_struct;

void Inc_PID_Operation(pid_struct*ctr);
void Pos_PID_Operation(pid_struct*ctr);

#endif

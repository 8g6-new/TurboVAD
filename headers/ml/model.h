#ifndef MODEL_H
#define MODEL_H

    #include <math.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdio.h>
    #include <string.h>

    #include "weights.h"

    typedef struct {
       float val;
       bool pred;
    } pred_t;
   
    void dot(const float input[4], const float W[16], float output[4]);
    void  softmax(float scores[4], float output[4]);
    float sigmoid(float x);
    pred_t  model(float input[8]);
    float weighted_sum(float mu, float sigma);

#endif

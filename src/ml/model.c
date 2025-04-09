#include "../../headers/ml/model.h"


void softmax(float scores[4], float output[4]) {
    float max_val = scores[0];

    for (size_t i = 1; i < 4; i++) {
        if (scores[i] > max_val) {
            max_val = scores[i];
        }
    }

    float sum_exp = 0.0;
    for (size_t i = 0; i < 4; i++) {
        output[i] = expf(scores[i] - max_val);
        sum_exp += output[i];
    }


    float inv_sum = 1.0f / sum_exp;
    for (size_t i = 0; i < 4; i++) {
        output[i] *= inv_sum;
    }
}


inline float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

void dot(const float input[4], const float W[16], float output[4]) {
    memset(output, 0, sizeof(float) * 4);  

    for (size_t j = 0; j < 4; j++) {  
        float inp_j = input[j];  
        for (size_t i = 0; i < 4; i++) {  
            output[i] += inp_j * W[j * 4 + i];
        }
    }
}


inline float weighted_sum(float mu, float sigma) {
    return (mu * weight_factors[0]) + (sigma * weight_factors[1]);
}

pred_t model(float input_features[8]) {
    pred_t pred = {0};


    float attn_scores_mu[4] = {0.0}, attn_scores_sigma[4] = {0.0};

    dot(input_features, mu_attention_weights, attn_scores_mu);
    dot(input_features + 4, sigma_attention_weights, attn_scores_sigma);

    float attn_weights_mu[4], attn_weights_sigma[4];


    softmax(attn_scores_mu, attn_weights_mu);
    softmax(attn_scores_sigma, attn_weights_sigma);


    float final_mu = attn_weights_mu[0] * mu_classifier_weights[0] +
                     attn_weights_mu[1] * mu_classifier_weights[1] +
                     attn_weights_mu[2] * mu_classifier_weights[2] +
                     attn_weights_mu[3] * mu_classifier_weights[3];

    float final_sigma = attn_weights_sigma[0] * sigma_classifier_weights[0] +
                        attn_weights_sigma[1] * sigma_classifier_weights[1] +
                        attn_weights_sigma[2] * sigma_classifier_weights[2] +
                        attn_weights_sigma[3] * sigma_classifier_weights[3];


    pred.val  = sigmoid(weighted_sum(final_mu, final_sigma));

    pred.pred = (pred.val > PRED_TH);

    return pred;
}

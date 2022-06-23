/*
 * Copyright (c) 2022 Linaro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Modulate the alpha-beta frame voltage into Space Vector PWM.
 *
 * @param v_alpha voltage alpha component obtained from inverse park step
 *
 * @param v_beta voltage beta component obtained from inverse park step
 * 
 * @param dc_u pointer to the voltage to be applied to the in phase u of motor
 *
 * @param dc_v pointer voltage to be applied to the in phase v of motor
 *
 * @param dc_v pointer voltage to be applied to the in phase v of motor
 *  
 */
void foc_svm_set(float v_alpha, float v_beta, float *dca, float *dcb, float *dcc);

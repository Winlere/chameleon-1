/**
 *
 * @file parsec/runtime_zlocality.c
 *
 * @copyright 2012-2017 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2023 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon PaRSEC CHAMELEON_Complex64_t kernel locality management
 *
 * @version 1.2.0
 * @author Mathieu Faverge
 * @date 2022-02-22
 * @precisions normal z -> s d c
 *
 */
#include "chameleon_parsec.h"

void RUNTIME_zlocality_allrestrict( uint32_t where )
{
    (void)where;
    chameleon_warning("RUNTIME_zlocality_allrestrict(PaRSEC)", "Kernel locality cannot be specified with PaRSEC");
}

void RUNTIME_zlocality_onerestrict( cham_tasktype_t kernel, uint32_t where )
{
    (void)kernel;
    (void)where;
    chameleon_warning("RUNTIME_zlocality_onerestrict(PaRSEC)", "Kernel locality cannot be specified with PaRSEC");
}

void RUNTIME_zlocality_allrestore( )
{
    chameleon_warning("RUNTIME_zlocality_allrestore(PaRSEC)", "Kernel locality cannot be specified with PaRSEC");
}

void RUNTIME_zlocality_onerestore( cham_tasktype_t kernel )
{
    (void)kernel;
    chameleon_warning("RUNTIME_zlocality_onerestore(PaRSEC)", "Kernel locality cannot be specified with PaRSEC");
}

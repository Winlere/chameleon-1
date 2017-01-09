#
# Check timing/
#

set(TEST_CMD_shm    --n_range=500:2000:500 --nb=320 )
set(TEST_CMD_shmgpu --n_range=500:2000:500 --nb=320 --gpus=1)
set(TEST_CMD_mpi    --n_range=500:2000:500 --nb=320 --p=2)
set(TEST_CMD_mpigpu --n_range=500:2000:500 --nb=320 --p=2 --gpus=1)

set(MPI_CMD_shm )
set(MPI_CMD_shmgpu )
set(MPI_CMD_mpi mpirun -np 4)
set(MPI_CMD_mpigpu mpirun -np 4)

set( TEST_CATEGORIES shm )
if (CHAMELEON_USE_CUDA AND CUDA_FOUND)
   set( TEST_CATEGORIES ${TEST_CATEGORIES} shmgpu )
endif()

set(TESTLIST 
    gels
    gemm
    getrf_incpiv
    getrf_nopiv
    geqrf
    gelqf
    posv
    potrf
    potri
    )

set(CHAMELEON_PRECISIONS_ZC "c;z")
set(TESTLIST_ZC
    sytrf
    )

foreach(cat ${TEST_CATEGORIES})
    foreach(prec ${RP_CHAMELEON_PRECISIONS})
        string(TOUPPER ${prec} PREC)

        if (CHAMELEON_PREC_${PREC})
            foreach(test ${TESTLIST})
                add_test(time_${cat}_${prec}${test} ${MPI_CMD_${cat}} ./time_${prec}${test}_tile ${TEST_CMD_${cat}} --check --warmup)
            endforeach()
        endif()
    endforeach()
    foreach(prec ${CHAMELEON_PRECISIONS_ZC})
        string(TOUPPER ${prec} PREC)

        if (CHAMELEON_PREC_${PREC})
            foreach(test ${TESTLIST_ZC})
                add_test(time_${cat}_${prec}${test} ${MPI_CMD_${cat}} ./time_${prec}${test}_tile ${TEST_CMD_${cat}} --check --warmup)
            endforeach()
        endif()
    endforeach()
endforeach()

if (CHAMELEON_USE_MPI AND MPI_C_FOUND)
    set( TEST_CATEGORIES mpi )
    set( TEST_CMD_mpi    --p=2 --n_range=2000:2000:1)
    set( TEST_CMD_mpigpu --p=2 --n_range=2000:2000:1 --gpus=1)
    #set( TEST_CATEGORIES ${TEST_CATEGORIES} mpi )
    #if (CHAMELEON_USE_CUDA AND CUDA_FOUND)
    #    set( TEST_CATEGORIES ${TEST_CATEGORIES} mpigpu )
    #endif()
    set(TESTLIST_MPI
        potrf
        )
    foreach(cat ${TEST_CATEGORIES})
        foreach(prec ${RP_CHAMELEON_PRECISIONS})
            string(TOUPPER ${prec} PREC)

            if (CHAMELEON_PREC_${PREC})
                foreach(test ${TESTLIST_MPI})
                    add_test(time_${cat}_${prec}${test} ${MPI_CMD_${cat}} ./time_${prec}${test}_tile ${TEST_CMD_${cat}} --check --warmup)
                endforeach()
            endif()
        endforeach()
    endforeach()
endif()


@ECHO OFF
IF [%1]==[all] GOTO ALL
IF [%1]==[report] GOTO REPORT
mpiexec -n 4 MPI.exe -v 1 -proc 4 -rows 3 -iter 1000 -pre 0.0001 -in mpi_test.txt -gen 0 -out mpi_test_result.txt
GOTO DONE

:ALL
set perIteration=10
for %%i in (10 100 200 400 800 1600 3200 4800 6400 8000) do (
    echo size = %%i
    echo 1 host
	set /a counter=0
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		mpiexec -n 1 MPI.exe -v 0 -proc 1 -rows %%i -iter 10000 -pre 0.0000001 		
	)

    echo 2 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j 
		mpiexec -n 2 MPI.exe -v 0 -proc 2 -rows %%i -iter 10000 -pre 0.0000001
	)
    echo 4 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		mpiexec -n 4 MPI.exe -v 0 -proc 4 -rows %%i -iter 10000 -pre 0.0000001 
	)
)
GOTO DONE
:REPORT 
php report.php
GOTO DONE

:DONE
@PAUSE
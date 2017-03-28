@ECHO OFF
IF [%1]==[all] GOTO ALL
IF [%1]==[report] GOTO REPORT
mpiexec -n 1 MPI_QuickSort.exe -v 1 -proc 4 -size 10000 -gen 1
GOTO DONE

:ALL
set perIteration=10
for %%i in (100 1000 10000 100000 1000000 10000000) do (
    echo size = %%i
    echo 1 host
	set /a counter=0
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		mpiexec -n 1 MPI_QuickSort.exe -v 0 -proc 1 -size %%i -gen 1	
	)

    echo 2 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j 
		mpiexec -n 2 MPI_QuickSort.exe -v 0 -proc 2 -size %%i -gen 1
	)
    echo 4 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		mpiexec -n 4 MPI_QuickSort.exe -v 0 -proc 4 -size %%i -gen 1
	)
)
GOTO DONE
:REPORT 
php qsort_report.php
GOTO DONE

:DONE
@PAUSE
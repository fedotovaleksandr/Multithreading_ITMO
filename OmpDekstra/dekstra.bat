@ECHO OFF
IF [%1]==[all] GOTO ALL
IF [%1]==[report] GOTO REPORT
OmpDekstra.exe -out dekstra_test_report.txt -in dekstra_test.txt -threads 4 -v 1 -start_node 1
GOTO DONE

:ALL
set perIteration=10
for %%i in (10 100 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000) do (
    echo size = %%i
    echo 1 host
	set /a counter=0
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		OmpDekstra.exe -out out_deksatra.txt -gen %%i -threads 1 -v 0 -start_node 1
	)

    echo 2 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j 
		OmpDekstra.exe -out out_deksatra.txt -gen %%i -threads 2 -v 0 -start_node 1
	)
    echo 4 hosts
    for /L %%j in (1,1,%perIteration%) do (
        echo iteration %%j
		OmpDekstra.exe -out out_deksatra.txt -gen %%i -threads 4 -v 0 -start_node 1
	)
)
GOTO DONE
:REPORT 
php dekstra_report.php
GOTO DONE

:DONE
@PAUSE
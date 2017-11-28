<?php
$data=[];
$filename = isset($argv[1]) ? $argv[1] : 'jacoby_report.txt';
$filenameOut = isset($argv[2]) ? $argv[2] : 'jacoby_report.csv';
$delimetr = ';';
$handle = fopen($filename, 'r');
if ($handle) {
    while (($line = fgets($handle)) !== false) {
        $matches=[];		
        preg_match('/.*_(\d+)x\d+__proccessor_(\d+): (\d+)ms.*/i',$line,$matches);
        if (!empty($matches)){
            $processors = $matches[2];
            $time = $matches[3];
            $size = $matches[1];
            $data[$processors][$size][] = $time;
        }; 
    }     
    fclose($handle);
}; 
$head = array_merge(['proc/size'],array_keys($data[1]));
file_put_contents($filenameOut,join($delimetr,$head));
foreach ($data as $proc=>$dataProc) {
	$dataSize=[];
	foreach ($dataProc as $size=>$times){
		$dataSize[$size] =  array_sum($times) / count($times); 
        $dataSize[$size] = str_replace('.',',',strval($dataSize[$size]));
	}
    $lineData = array_merge(["\nproc:{$proc}"],array_values($dataSize));	
    file_put_contents($filenameOut,join($delimetr,$lineData),FILE_APPEND);
	};
?>
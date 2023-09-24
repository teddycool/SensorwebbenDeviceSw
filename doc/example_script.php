<?php
/*
 * For the receiving to work from the sensor in url-mode,
 * we need to get the parameters from the url and do something with it.
 * This example shows a very simple receiving-script, takeing the 
 * values and store them in a database.
*/
require_once('config.php');
//Read values from request message
$chipid         = isset($_GET['chipid'])  ? $_GET['chipid'] : '';
$measurement    = isset($_GET['measm'])   ? $_GET['measm'] 	: 'null';
$value          = isset($_GET['value'])   ? $_GET['value'] 	: 'null';
$unit           = isset($_GET['unit'])    ? $_GET['unit'] 	: 'null';


if ($chipid==''){
    echo "Unsupported box: " . $chipid;
}
else {
    $sql = "INSERT INTO Value (chipid, measure, unit, value, time) "
        . "VALUES ('{$chipid}', '{$measurement}','{$unit}', $value,  Now())";
            // Create connection
        $conn = new mysqli(DB_HOST, DB_USER, DB_PASSWORD, DB_DATABASE);
        // Check connection
       if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        }     
           //Run each insertstring from array...
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully: " . 'sqlinsert...';
        } 
        else 
        {            
            //TODO: insert into errorlog..
            $Error = $sql . " " . $conn->error;
            echo $Error;            
        }           
        
    $conn->close();
}

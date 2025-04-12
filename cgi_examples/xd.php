#!/usr/bin/php-cgi
<?php
// Basic form handler that echoes back POST data
header("Content-Type: text/html");

// Print environment variables for debugging
echo "<h1>PHP CGI Test</h1>";
echo "<h2>Environment Variables</h2>";
echo "<pre>";
foreach($_SERVER as $key => $value) {
    echo "$key = $value\n";
}
echo "</pre>";

// Handle POST data
echo "<h2>POST Data</h2>";
echo "<pre>";
if(!empty($_POST)) {
    foreach($_POST as $key => $value) {
        echo "$key = $value\n";
    }
} else {
    echo "No POST data received\n";
}
echo "</pre>";

// Handle file uploads
echo "<h2>File Uploads</h2>";
if(!empty($_FILES)) {
    foreach($_FILES as $file) {
        echo "File: {$file['name']}<br>";
        echo "Type: {$file['type']}<br>";
        echo "Size: {$file['size']} bytes<br>";
        
        // Move the uploaded file to a permanent location
        if(move_uploaded_file($file['tmp_name'], "/tmp/{$file['name']}")) {
            echo "File saved to /tmp/{$file['name']}<br>";
        } else {
            echo "Error saving file<br>";
        }
    }
} else {
    echo "No files uploaded";
}
?>
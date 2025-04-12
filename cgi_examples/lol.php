<?php
#!/usr/bin/php-cgi

// Set content type
header("Content-Type: text/html");
?>
<!DOCTYPE html>
<html>
<head>
    <title>PHP CGI Upload Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 20px auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            background-color: white;
            border-radius: 5px;
            padding: 20px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input[type="text"], textarea {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        pre {
            background-color: #f8f8f8;
            padding: 10px;
            border-radius: 4px;
            overflow: auto;
        }
        .results {
            margin-top: 30px;
            border-top: 1px solid #ddd;
            padding-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>PHP CGI Upload Test</h1>

        <!-- Upload Form -->
        <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST" enctype="multipart/form-data">
            <div class="form-group">
                <label for="username">Your Name:</label>
                <input type="text" name="username" id="username">
            </div>
            
            <div class="form-group">
                <label for="message">Message:</label>
                <textarea name="message" id="message" rows="4"></textarea>
            </div>
            
            <div class="form-group">
                <label for="userfile">Select File to Upload:</label>
                <input type="file" name="userfile" id="userfile">
            </div>
            
            <input type="submit" value="Upload File">
        </form>

<?php
// Process form submission
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo '<div class="results">';
    
    // Show environment variables
    echo "<h2>Environment Variables</h2>";
    echo "<pre>";
    foreach($_SERVER as $key => $value) {
        echo htmlspecialchars("$key = $value") . "\n";
    }
    echo "</pre>";

    // Show POST data
    echo "<h2>POST Data</h2>";
    echo "<pre>";
    if(!empty($_POST)) {
        foreach($_POST as $key => $value) {
            echo htmlspecialchars("$key = $value") . "\n";
        }
    } else {
        echo "No POST data received\n";
    }
    echo "</pre>";

    // Handle file upload
    echo "<h2>File Upload Results</h2>";
    if(!empty($_FILES)) {
        foreach($_FILES as $fieldname => $file) {
            echo "<p>";
            echo "Field name: " . htmlspecialchars($fieldname) . "<br>";
            echo "File name: " . htmlspecialchars($file['name']) . "<br>";
            echo "File type: " . htmlspecialchars($file['type']) . "<br>";
            echo "File size: " . htmlspecialchars($file['size']) . " bytes<br>";
            
            if($file['error'] === UPLOAD_ERR_OK) {
                $upload_dir = "/tmp";
                $dest_path = $upload_dir . "/" . basename($file['name']);
                
                if(move_uploaded_file($file['tmp_name'], $dest_path)) {
                    echo "<strong>File successfully uploaded to $dest_path</strong><br>";
                } else {
                    echo "<strong>Error moving uploaded file!</strong><br>";
                }
            } else {
                echo "Error code: " . htmlspecialchars($file['error']) . "<br>";
                
                // Translate error codes
                $upload_errors = array(
                    UPLOAD_ERR_INI_SIZE => 'File exceeds upload_max_filesize directive in php.ini',
                    UPLOAD_ERR_FORM_SIZE => 'File exceeds MAX_FILE_SIZE directive in HTML form',
                    UPLOAD_ERR_PARTIAL => 'File was only partially uploaded',
                    UPLOAD_ERR_NO_FILE => 'No file was uploaded',
                    UPLOAD_ERR_NO_TMP_DIR => 'Missing a temporary folder',
                    UPLOAD_ERR_CANT_WRITE => 'Failed to write file to disk',
                    UPLOAD_ERR_EXTENSION => 'File upload stopped by extension'
                );
                
                echo "Error message: " . $upload_errors[$file['error']] . "<br>";
            }
            echo "</p>";
        }
    } else {
        echo "<p>No files uploaded.</p>";
    }
    
    echo '<hr><p><a href="' . $_SERVER['PHP_SELF'] . '">Try another upload</a></p>';
    echo '</div>';
}
?>
    </div>
</body>
</html>
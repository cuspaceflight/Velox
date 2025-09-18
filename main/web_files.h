#pragma once

static const char* main_page = R"RAWSTRING(
<!DOCTYPE html>
<html>
    <head>
        <title>Velox ground station</title>
    </head>
    <body>
        <div>
            <h1>Velox</h1>
            <h2>TIME: <span id="TIME"></span></h2>
            <h2>ENTRY: <span id="ENTR"></span></h2>
            <h1>GPS</h1>
            <h2>LAT: <span id="GLAT"></span></h2>
            <h2>LON: <span id="GLON"></span></h2>
            <h2>ALT: <span id="GALT"></span></h2>
            <h1>BMP</h1>
            <h2>PRESSURE   : <span id="BPRE"></span></h2>
            <h2>TEMPERATURE: <span id="BTEM"></span></h2>
            <h1>MPU</h1>
            <h2>ACCEL X: <span id="MACX"></span></h2>
            <h2>ACCEL Y: <span id="MACY"></span></h2>
            <h2>ACCEL Z: <span id="MACZ"></span></h2>
            <h2>GYRO  X: <span id="MGYX"></span></h2>
            <h2>GYRO  Y: <span id="MGYY"></span></h2>
            <h2>GYRO  Z: <span id="MGYZ"></span></h2>
            <h2>TEMP   : <span id="MTEM"></span></h2>

        </div>

        <script>
            setInterval(function() {
                getData();
            }, 5000);

            function getData() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        console.log(this.responseText);
                        data = JSON.parse(this.responseText);
                        console.log(data);

                        document.getElementById("TIME").innerHTML = data["TIME"]
                        document.getElementById("ENTR").innerHTML = data["ENTR"]
                        document.getElementById("GLAT").innerHTML = data["GLAT"]
                        document.getElementById("GLON").innerHTML = data["GLON"]
                        document.getElementById("GALT").innerHTML = data["GALT"]
                        document.getElementById("BPRE").innerHTML = data["BPRE"]
                        document.getElementById("BTEM").innerHTML = data["BTEM"]
                        document.getElementById("MACX").innerHTML = data["MACX"]
                        document.getElementById("MACY").innerHTML = data["MACY"]
                        document.getElementById("MACZ").innerHTML = data["MACZ"]
                        document.getElementById("MGYX").innerHTML = data["MGYX"]
                        document.getElementById("MGYY").innerHTML = data["MGYY"]
                        document.getElementById("MGYZ").innerHTML = data["MGYZ"]
                        document.getElementById("MTEM").innerHTML = data["MTEM"] }
                }
                xhttp.open("GET", "data", true);
                xhttp.send();
            }
        </script>
    </body>
</html>)RAWSTRING";

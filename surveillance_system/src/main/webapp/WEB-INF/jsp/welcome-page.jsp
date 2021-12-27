<%@ taglib prefix="form" uri="http://www.springframework.org/tags/form" %>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<%@taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core"%>
<html>
<head>
    <title>Surveillance Online System</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-wEmeIV1mKuiNpC+IOBjI7aAzPcEZeedi5yW5f2yOq55WWLwNGmvvx4Um1vskeMj0" crossorigin="anonymous">

</head>
<body>

<div class="container">
    <nav class="navbar navbar-expand-lg navbar-light navbar navbar-dark bg-success shadow">
        <a style="padding-left: 20px;" class="navbar-brand" href="/"><h1>Surveillance System</h1></a>
    </nav>
    <br>
    <br>
    <div class="row">
        <div class="col-2"></div>
        <div class="col-4 text-center">
            <div class="card shadow" style=width:25rem;">
                <img src="/img/User.png" class="card-img-top" alt="User-Icon">
                <div class="card-title">
                    <h3 class="card-title">User Login Page</h3>
                </div>
                <div class="card-body">
                    <a class="btn-lg btn-success" href="/user">Login</a>
                </div>
            </div>
        </div>
        <div class="col-4 text-center">
            <div class="card shadow " style="width: 25rem;">
                <img src="/img/Admin.png" class="card-img-top" alt="User-Icon">
                <div class="card-title">
                    <h3 class="card-title">Admin Login Page</h3>
                </div>
                <div class="card-body">
                    <a class="btn-lg btn-success" href="/admin">Login</a>
                </div>
            </div>
        </div>
        <div class="col-2"></div>
    </div>
</div>



<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0/dist/js/bootstrap.bundle.min.js" integrity="sha384-p34f1UUtsS3wqzfto5wAAmdvj+osOnFyQFpp4Ua3gs/ZVWx6oOypYoCJhGGScy+8" crossorigin="anonymous"></script>
</body>
</html>

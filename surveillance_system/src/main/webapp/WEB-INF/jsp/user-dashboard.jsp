<%@ taglib prefix="form" uri="http://www.springframework.org/tags/form" %>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<%@taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core"%>
<html>
<head>
    <META HTTP-EQUIV="Refresh" CONTENT="5">
    <title>Surveillance Online System</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-wEmeIV1mKuiNpC+IOBjI7aAzPcEZeedi5yW5f2yOq55WWLwNGmvvx4Um1vskeMj0" crossorigin="anonymous">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css" />
    <link rel="stylesheet" href="/css/styles.css" />
</head>


</head>

<body>
<div class="d-flex" id="wrapper">
    <!-- Sidebar -->
    <div class="bg-white" id="sidebar-wrapper">
        <div class="sidebar-heading text-center py-4 primary-text fs-4 fw-bold text-uppercase border-bottom"><i
                class="fas fa-user me-2"></i>User Panel</div>
        <div class="list-group list-group-flush my-3">
            <a href="/admin" class="list-group-item list-group-item-action bg-transparent second-text active"><i
                    class="fas fa-tachometer-alt me-2"></i>Dashboard</a>
            <a href="/logout" class="list-group-item list-group-item-action bg-transparent text-danger fw-bold"><i
                    class="fas fa-power-off me-2"></i>Logout</a>
        </div>
    </div>
    <!-- /#sidebar-wrapper -->

    <!-- Page Content -->
    <div id="page-content-wrapper">
        <nav class="navbar navbar-expand-lg navbar-light bg-transparent py-4 px-4">
            <div class="d-flex align-items-center">
                <i class="fas fa-align-left primary-text fs-4 me-3" id="menu-toggle"></i>
                <h2 class="fs-2 m-0">Dashboard</h2>
            </div>

            <button class="navbar-toggler" type="button" data-bs-toggle="collapse"
                    data-bs-target="#navbarSupportedContent" aria-controls="navbarSupportedContent"
                    aria-expanded="false" aria-label="Toggle navigation">
                <span class="navbar-toggler-icon"></span>
            </button>

            <div class="collapse navbar-collapse" id="navbarSupportedContent">
                <ul class="navbar-nav ms-auto mb-2 mb-lg-0">
                    <li class="nav-item dropdown">
                        <a class="nav-link dropdown-toggle second-text fw-bold" href="#" id="navbarDropdown"
                           role="button" data-bs-toggle="dropdown" aria-expanded="false">
                            <i class="fas fa-user me-2"></i>${userList.get(0)[1]}
                        </a>
                        <ul class="dropdown-menu" aria-labelledby="navbarDropdown">
                            <li><a class="dropdown-item" href="#">Logout</a></li>
                        </ul>
                    </li>
                </ul>
            </div>
        </nav>

        <div class="container-fluid px-4">
            <div class="row g-3 my-2">
                <div class="col-md-3">
                    <div class="p-3 bg-white shadow-sm d-flex justify-content-around align-items-center rounded">
                        <div>
                            <p class="fs-5">Contract expires in 24 days</p>
                        </div>
                        <i class="fas fa-info-circle fs-1 primary-text border rounded-full secondary-bg p-3"></i>
                    </div>
                </div>
            </div>

            <div class="row my-5">
                <h3 class="fs-4 mb-3">Activated Products</h3>
                <div class="col">
                    <div class="row text-center" >
                        <div style="margin-bottom: 15px;" class="p-1 bg-white shadow-sm d-flex justify-content-around align-items-center rounded">


                            <div class="col-1">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">ID</p>
                            </div>
                            <div class="col-2">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">User</p>
                            </div>
                            <div class="col-2">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">Contract</p>
                            </div>
                            <div class="col-2">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">Armed</p>
                            </div>
                            <div class="col-3">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">Mac Address</p>
                            </div>
                            <div class="col-2">
                                <p style="font-weight:bold; padding-top: 10px;" class="fs-5">Alarm</p>
                            </div>

                        </div>
                    </div>


                    <c:forEach items="${userList}" var="user">
                        <div class="row text-center">
                            <div style="margin-bottom: 10px;" class="p-1 bg-white shadow-sm d-flex justify-content-around align-items-center rounded">
                                <div class="col-1">
                                    <p style="padding-top: 10px;" class="fs-5">${user[0]}</p>
                                </div>
                                <div class="col-2">
                                    <p style="padding-top: 10px;" class="fs-5">${user[1]}</p>
                                </div>
                                <div class="col-2">
                                    <c:choose>
                                        <c:when test="${user[2] == 'false'}">
                                            <p style="padding-top: 10px;" class="fs-5">Expired</p>
                                        </c:when>
                                        <c:otherwise>
                                            <p style="padding-top: 10px;" class="fs-5">Active</p>
                                        </c:otherwise>
                                    </c:choose>

                                </div>
                                <div class="col-2">
                                    <!-- <p style="padding-top: 10px;" class="fs-5">${user[3]}</p> -->
                                    <c:choose>
                                        <c:when test="${user[3] == 'false'}">
                                                <span style="color: Tomato;">
                                                     <a href="/user/${user[1]}/${user[4]}/${user[3]}/control" style="color: Tomato">
                                                        <i class="fas fa-power-off fs-1 rounded-full "></i>
                                                     </a>
                                                </span>
                                        </c:when>
                                        <c:otherwise>
                                                <span style="color: green;">
                                                    <a href="/user/${user[1]}/${user[4]}/${user[3]}/control">
                                                        <i class="fas fa-power-off fs-1 rounded-full"></i>
                                                    </a>
                                                </span>
                                        </c:otherwise>
                                    </c:choose>
                                </div>
                                <div class="col-3">
                                    <p style="padding-top: 10px;" class="fs-5">${user[4]}</p>
                                </div>
                                <div class="col-2">
                                    <c:choose>
                                    <c:when test="${user[5] == 'false'}">
                                                <span>
                                                    <i class="fas fa-exclamation-circle fs-1 second-text rounded-full "></i>
                                                </span>
                                    </c:when>
                                    <c:otherwise>
                                    <span style="color: Tomato;">
                                                   <i class="fas fa-exclamation-circle fs-1 rounded-full"></i>
                                            </c:otherwise>
                                        </c:choose>
                                </div>
                            </div>
                        </div>
                    </c:forEach>



                </div>
            </div>

        </div>
    </div>
</div>
<!-- /#page-content-wrapper -->
</div>


<script>
    var el = document.getElementById("wrapper");
    var toggleButton = document.getElementById("menu-toggle");

    toggleButton.onclick = function () {
        el.classList.toggle("toggled");
    };
</script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/js/bootstrap.bundle.min.js"></script>
</body>





<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0/dist/js/bootstrap.bundle.min.js" integrity="sha384-p34f1UUtsS3wqzfto5wAAmdvj+osOnFyQFpp4Ua3gs/ZVWx6oOypYoCJhGGScy+8" crossorigin="anonymous"></script>
</body>
</html>

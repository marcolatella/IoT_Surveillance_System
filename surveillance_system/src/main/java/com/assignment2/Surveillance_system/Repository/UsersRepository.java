package com.assignment2.Surveillance_system.Repository;

import com.assignment2.Surveillance_system.Model.User;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;


@Repository("userRepository")
public interface UsersRepository extends JpaRepository<User, String> {

    Optional<User> findUserByUsername(String username);

    @Query("Select u.id, u.username, u.enabled, u.sys_on, u.mac, u.alarm_on, u.connected from User as u")
    List<Object[]> findAllUsers();

    @Query("Select u.id, u.username, u.enabled, u.sys_on, u.mac, u.alarm_on, u.connected from User as u where u.username=:username")
    List<Object[]> findUserByUsr(@Param("username") String username);

    User getUserByMac(String mac);

    Boolean existsUserByMac(String mac);


}
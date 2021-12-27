package com.assignment2.Surveillance_system.Controller;

import com.assignment2.Surveillance_system.Model.User;
import com.assignment2.Surveillance_system.Service.UserDetailsServiceImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.ModelAndView;

@RestController
public class NodesController {

    @Autowired
    UserDetailsServiceImpl userDetails;

    @GetMapping("/surv-sys/{userID}/{macAddr}/config")
    public String config(@PathVariable String userID, @PathVariable String macAddr){
        User tmp = new User("User_"+userID,"Pass_"+userID,true,
                "ROLE_USER",false,macAddr, false, true);
        userDetails.addUser(tmp);
        return "User Configured";
    }

    @GetMapping("/surv-sys/{userID}/{macAddr}/{armed}/config")
    public String setArmed(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean armed){
        User tmp = userDetails.getUser(macAddr);
        tmp.setSys_on(armed);
        userDetails.addUser(tmp);
        return "User: " + macAddr + " Armed";
    }

    @GetMapping("/surv-sys/{userID}/{macAddr}/{connection}/connection")
    public String setConnection(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean connection){
        if(!userDetails.isPresent(macAddr)){
            return "No User: " + macAddr + "exists";
        }
        User tmp = userDetails.getUser(macAddr);
        tmp.setAlarm_on(false);
        tmp.setSys_on(false);
        tmp.setConnected(connection);
        userDetails.addUser(tmp);
        return "User: " + macAddr + " Connection: " + connection;
    }

    @GetMapping("/surv-sys/{userID}/{macAddr}/{alarm}/alarm")
    public String setAlarm(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean alarm){
        User tmp = userDetails.getUser(macAddr);
        tmp.setAlarm_on(alarm);
        userDetails.addUser(tmp);
        return "User: " + macAddr + " Alarm setted: " + alarm;
    }


}

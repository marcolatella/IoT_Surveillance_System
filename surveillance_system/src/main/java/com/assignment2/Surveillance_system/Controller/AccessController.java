package com.assignment2.Surveillance_system.Controller;


import com.assignment2.Surveillance_system.Model.User;
import com.assignment2.Surveillance_system.Service.UserDetailsServiceImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.servlet.ModelAndView;

import java.util.List;
import java.util.concurrent.Delayed;

@RestController
public class AccessController {

    String InterpretIP = "192.168.1.5";

    @Autowired
    UserDetailsServiceImpl userDetails;

    @GetMapping("/")
    public ModelAndView home(){
        return new ModelAndView("welcome-page");
    }

    @GetMapping("/user")
    public ModelAndView userHome(Authentication authentication){
        ModelAndView mav = new ModelAndView();
        mav.setViewName("user-dashboard");
        UserDetails authUser = (UserDetails) authentication.getPrincipal();
        List<Object[]> user = userDetails.findUserByUsr(authUser.getUsername());
        mav.addObject("userList", user);
        return mav;
    }

    @GetMapping("/admin")
    public ModelAndView adminHome(){
        int activeU = 0;
        int activeS = 0;
        int connectedS = 0;
        ModelAndView mav = new ModelAndView();
        List<Object[]> users = userDetails.findAllUsers();
        users.remove(0);
        for(Object[] usr : users){
            if((Boolean)usr[2])
                activeU++;
            if((Boolean)usr[3])
                activeS++;
            if((Boolean)usr[6])
                connectedS++;
        }
        int nUsers = users.size();
        mav.setViewName("admin-dashboard");
        mav.addObject("nUsers", nUsers);
        mav.addObject("usersList", users);
        mav.addObject("activeUsers", activeU);
        mav.addObject("activeSystems", activeS);
        mav.addObject("connectedSystems", connectedS);
        return mav;
    }


    @GetMapping("/admin/{userID}/{macAddr}/{armed}/control")
    public ModelAndView postAdminRequest(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean armed) throws InterruptedException {
        final String uri = "http://"+ InterpretIP +"/nodes/control";
        RestTemplate restTemplate = new RestTemplate();
        String payload = userID + " " + macAddr + " " + !armed;
        String result = restTemplate.postForObject(uri,payload,String.class);
        System.out.println(result);
        Thread.sleep(1500);
        return new ModelAndView("redirect:/admin");
    }

    @GetMapping("/admin/{userID}/{macAddr}/{contract}/contract")
    public ModelAndView postAdminRequestContract(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean contract) throws InterruptedException {
        final String uri = "http://"+ InterpretIP +"/nodes/contract";
        RestTemplate restTemplate = new RestTemplate();
        String payload = userID + " " + macAddr + " " + !contract;
        String result = restTemplate.postForObject(uri,payload,String.class);
        User tmp = userDetails.getUser(macAddr);
        tmp.setEnabled(!contract);
        userDetails.addUser(tmp);
        System.out.println(result);
        Thread.sleep(1500);
        return new ModelAndView("redirect:/admin");
    }

    @GetMapping("/admin/{userID}/{macAddr}/reset-rfid")
    public ModelAndView postAdminRequestContract(@PathVariable String userID, @PathVariable String macAddr) throws InterruptedException {
        final String uri = "http://"+ InterpretIP +"/nodes/reset";
        RestTemplate restTemplate = new RestTemplate();
        String payload = macAddr;
        String result = restTemplate.postForObject(uri,payload,String.class);
        return new ModelAndView("redirect:/admin");
    }

    @GetMapping("/user/{userID}/{macAddr}/{armed}/control")
    public ModelAndView postUserRequest(@PathVariable String userID, @PathVariable String macAddr, @PathVariable boolean armed) throws InterruptedException {
        final String uri = "http:// "+ InterpretIP + "/nodes/control";
        RestTemplate restTemplate = new RestTemplate();
        String payload = userID + " " + macAddr + " " + !armed;
        String result = restTemplate.postForObject(uri,payload,String.class);
        System.out.println(result);
        Thread.sleep(1500);
        return new ModelAndView("redirect:/user");
    }





}

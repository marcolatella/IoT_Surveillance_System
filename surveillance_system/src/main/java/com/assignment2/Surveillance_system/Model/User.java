package com.assignment2.Surveillance_system.Model;

import javax.persistence.*;

@Entity
@Table(name = "User")
public class User{

    @Id
    @Column(name = "mac", nullable = false)
    private String mac;
    @GeneratedValue(strategy = GenerationType.AUTO)
    private int id;
    private String username;
    private String password;
    private boolean enabled;
    private String roles;
    private boolean sys_on;
    private boolean alarm_on;
    private boolean connected;


    public User(String username, String password, boolean enabled, String roles,
                boolean sys_on, String mac, boolean alarm_on, boolean connected) {
        this.username = username;
        this.password = password;
        this.enabled = enabled;
        this.roles = roles;
        this.sys_on = sys_on;
        this.mac = mac;
        this.alarm_on = alarm_on;
        this.connected = connected;
    }

    public User() {
    }


    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public String getRoles() {
        return roles;
    }

    public boolean isSys_on() {
        return sys_on;
    }

    public boolean isConnected() {
        return connected;
    }

    public void setConnected(boolean connected) {
        this.connected = connected;
    }

    public boolean isAlarm_on() {
        return alarm_on;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public void setSys_on(boolean sys_on) {
        this.sys_on = sys_on;
    }

    public void setAlarm_on(boolean alarm_on) {
        this.alarm_on = alarm_on;
    }
}

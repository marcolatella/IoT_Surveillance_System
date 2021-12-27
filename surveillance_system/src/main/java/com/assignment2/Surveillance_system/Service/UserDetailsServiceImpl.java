package com.assignment2.Surveillance_system.Service;

import com.assignment2.Surveillance_system.Model.User;
import com.assignment2.Surveillance_system.Model.UserDetailsImpl;
import com.assignment2.Surveillance_system.Repository.UsersRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;


@Service("userDetailService")
public class UserDetailsServiceImpl implements UserDetailsService {

    @Autowired
    UsersRepository usersRepository;

    @Override
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
        Optional<User> user = usersRepository.findUserByUsername(username);

        user.orElseThrow(() -> new UsernameNotFoundException("Bad Credentials"));

        return user.map(UserDetailsImpl::new).get();
    }

    public List<Object[]> findAllUsers(){
        return usersRepository.findAllUsers();
    }

    public void addUser(User usr){
        usersRepository.save(usr);
    }

    public User getUser(String mac){
        return usersRepository.getUserByMac(mac);
    }

    public List<Object[]> findUserByUsr(String username){
        return usersRepository.findUserByUsr(username);
    }

    public boolean isPresent(String mac){
        return usersRepository.existsUserByMac(mac);
    }

}

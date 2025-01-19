package com.example.ultrasonicsensor.web;

import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/notifications")
public class Controller {


    private final JavaMailSender mailSender;

    public Controller(JavaMailSender mailSender) {
        this.mailSender = mailSender;
    }

    @PostMapping("/send")
    public String sendNotification(@RequestBody Notifications notifications) {

        SimpleMailMessage email = new SimpleMailMessage();
        email.setTo(notifications.getTo());
        email.setSubject(notifications.getSubject());
        email.setText(notifications.getMessage());
        email.setFrom("hgh807721@gmail.com");
        mailSender.send(email);
        System.out.println(email);
        return "Email Sent!";
    }
}

package com.example.sack;

public class UserModel {
    private String fullName;
    private String username;
    private int age;
    private String password;
    private String securityQuestion;
    private String securityAnswer;

    public UserModel(String fullName, String username, int age, String password, String securityQuestion, String securityAnswer) {
        this.fullName = fullName;
        this.username = username;
        this.age = age;
        this.password = password;
        this.securityQuestion = securityQuestion;
        this.securityAnswer = securityAnswer;
    }

    public String getFullName() {
        return fullName;
    }

    public String getUsername() {
        return username;
    }

    public int getAge() {
        return age;
    }

    public String getPassword() {
        return password;
    }

    public String getSecurityQuestion() {
        return securityQuestion;
    }

    public String getSecurityAnswer() {
        return securityAnswer;
    }
}

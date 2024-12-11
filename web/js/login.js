import authManager from './auth.js';

// 在页面加载时，检查 localStorage 中是否有存储的用户名和密码，并自动填充到表单中
document.addEventListener('DOMContentLoaded', function () {
    const savedUsername = localStorage.getItem('username');
    const savedPassword = localStorage.getItem('password');

    if (savedUsername && savedPassword) {
        document.getElementById('username').value = savedUsername;
        document.getElementById('password').value = savedPassword;
        document.getElementById('remember').checked = true;
    }
});

// 在登录表单提交时使用
document.getElementById('login_form').addEventListener('submit', function (e) {
    e.preventDefault();

    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const rememberMe = document.getElementById('remember').checked;

    async function validateLogin(username, password) {
        try {
            const response = await fetch('/user/login', {
                method: 'POST',
                credentials: 'include', // 确保请求中包含 cookies
                body: JSON.stringify({ username, password }),
                headers: {
                    'Content-Type': 'application/json'
                }
            });

            if (!response.ok) {
                throw new Error('登录失败');
            }

            // 假设 data.success 是服务器返回的一个布尔值，指示登录是否成功
            return response;
        } catch (error) {
            console.error(error);
            alert(error);
            return response; // 在发生错误时返回 false
        }
    }

    function deleteCookie(name) {
        document.cookie = name + '=; Max-Age=0; path=/';
    }

    // Usage
    // 在登录成功后，获取 'session_id' cookie
    validateLogin(username, password).then(response => {
        if (response.ok) {
            if (rememberMe) {
                localStorage.setItem('username', username);
                localStorage.setItem('password', password);
            } else {
                localStorage.removeItem('username');
                localStorage.removeItem('password');
            }
            window.location.href = 'index.html';
        } else {
            alert('登录失败！');
        }
    });
});
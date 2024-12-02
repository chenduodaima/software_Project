// software_Project/web/js/registerComponent.js

class RegisterComponent {
    constructor(formId) {
        this.form = document.getElementById(formId);
        this.usernameInput = this.form.querySelector('input[placeholder="Enter your username"]');
        this.accountInput = this.form.querySelector('input[placeholder="Enter your account"]');
        this.passwordInput = this.form.querySelector('input[placeholder="Enter your password"]');
        this.confirmPasswordInput = this.form.querySelector('input[placeholder="Confirm your password"]');
        
        this.form.addEventListener('submit', (event) => this.handleSubmit(event));
    }

    validateInputs() {
        const username = this.usernameInput.value.trim();
        const account = this.accountInput.value.trim();
        const password = this.passwordInput.value.trim();
        const confirmPassword = this.confirmPasswordInput.value.trim();

        if (!username || !account || !password || !confirmPassword) {
            alert('所有字段都是必填的。');
            return false;
        }

        if (password !== confirmPassword) {
            alert('密码不匹配。');
            return false;
        }
        
        fetch('/user/register', {
            method: 'POST',
            body: JSON.stringify({username, account, password }),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('注册失败')
            }
            return response.json()
        })
        .then(data => {
            return data.result;
        })
        .catch(error => {
            console.error(error);
            alert(error);
            return false;
        });
        return true;
    }

    handleSubmit(event) {
        event.preventDefault(); // 防止表单默认提交

        if (this.validateInputs()) {
            // 这里可以添加 AJAX 请求或其他处理逻辑
            alert('注册成功！');
            // 清空表单
            this.form.reset();
        }
    }
}

// 初始化注册组件
document.addEventListener('DOMContentLoaded', () => {
    new RegisterComponent('register_form'); // 确保这里的 id 是 register_form
});
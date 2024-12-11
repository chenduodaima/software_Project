class RegisterComponent {
    constructor(formId) {
        this.form = document.getElementById(formId);
        if (!this.form) {
            console.error(`Form with ID ${formId} not found.`);
            return;
        }
        this.nameInput = this.form.querySelector('input[placeholder="输入个人名称"]');
        this.usernameInput = this.form.querySelector('input[placeholder="输入用户名"]');
        this.passwordInput = this.form.querySelector('input[placeholder="输入密码"]');
        this.confirmPasswordInput = this.form.querySelector('input[placeholder="再次输入密码"]');
        
        this.form.addEventListener('submit', (event) => this.handleSubmit(event));
    }

    validateInputs() {
        const name = this.nameInput.value.trim();
        const username = this.usernameInput.value.trim();
        const password = this.passwordInput.value.trim();
        const confirmPassword = this.confirmPasswordInput.value.trim();
    
        if (!name || !username || !password || !confirmPassword) {
            alert('所有字段都是必填的。');
            return Promise.resolve(false);
        }
    
        if (password !== confirmPassword) {
            alert('密码不匹配。');
            return Promise.resolve(false);
        }
        
        return fetch('/user/register', {
            method: 'POST',
            body: JSON.stringify({name, username, password }),
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('注册失败');
            }
            console.log(true);
            return true;
        })
        .catch(error => {
            console.error(error);
            console.log(false);
            return false;
        });
    }
    
    handleSubmit(event) {
        event.preventDefault(); // 防止表单默认提交
        this.validateInputs().then(result => {
            console.log(result);
            if (result) {
                this.form.reset();
                alert('注册成功！');
            } else {
                alert('注册失败！');
            }
        });
    }

}

// 初始化注册组件
document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM fully loaded and parsed');
    new RegisterComponent('register_form');
});
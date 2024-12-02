class AuthManager {
    constructor() {
        // Cookie默认过期时间（7天）
        this.cookieExpireDays = 7;
    }

    // 设置Cookie
    setCookie(name, value, days = this.cookieExpireDays) {
        const date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        const expires = "expires=" + date.toUTCString();
        document.cookie = name + "=" + value + ";" + expires + ";path=/";
    }

    // 获取Cookie
    getCookie(name) {
        const cookieName = name + "=";
        const cookies = document.cookie.split(';');
        for(let i = 0; i < cookies.length; i++) {
            let cookie = cookies[i].trim();
            if (cookie.indexOf(cookieName) === 0) {
                return cookie.substring(cookieName.length, cookie.length);
            }
        }
        return "";
    }

    // 删除Cookie
    deleteCookie(name) {
        this.setCookie(name, "", -1);
    }

    // 保存登录状态到Cookie
    saveLoginState(username, rememberMe = false) {
        if (rememberMe) {
            this.setCookie("username", username);
            // 注意：出于安全考虑，不建议将密码存储在Cookie中
            this.setCookie("isLoggedIn", "true");
        }
        // 设置会话状态
        sessionStorage.setItem("username", username);
        sessionStorage.setItem("isLoggedIn", "true");
    }

    // 检查是否已登录
    isLoggedIn() {
        // 优先检查会话存储
        if (sessionStorage.getItem("isLoggedIn") === "true") {
            return true;
        }
        // 如果会话中没有，检查Cookie
        return this.getCookie("isLoggedIn") === "true";
    }

    // 获取当前登录用户
    getCurrentUser() {
        // 优先从会话存储中获取
        const sessionUser = sessionStorage.getItem("username");
        if (sessionUser) {
            return sessionUser;
        }
        // 如果会话中没有，从Cookie中获取
        return this.getCookie("username");
    }

    // 登出
    logout() {
        // 清除会话存储
        sessionStorage.removeItem("username");
        sessionStorage.removeItem("session_id");
        sessionStorage.removeItem("isLoggedIn");
        
        // 清除Cookie
        this.deleteCookie("username");
        this.deleteCookie("session_id");
        this.deleteCookie("isLoggedIn");
    }
}

// 创建并导出实例
const authManager = new AuthManager();
export default authManager;
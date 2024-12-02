const { ref, reactive, onMounted } = Vue;

const personal = Vue.createApp({
    setup() {
        // 导航栏
        const navItems = ref([
            { page: 'profile', label: '个人资料' },
            { page: 'security', label: '账号安全' },
            { page: 'logout', label: '退出登录' },
            // Add other nav items here
        ]);

        // 当前页面
        const activePage = ref('profile');
        const isEditing = ref(false);

        const switchPage = (page) => {
            activePage.value = page;
        };

        // 导航栏点击事件
        const handleNavClick = (page) => {
            isEditing.value = false;
            if (page === 'logout') {
                if (confirm('确定要退出登录吗？')) {
                    window.location.href = 'login.html';
                }
            } else {
                switchPage(page);
            }
        };

        //右侧编辑栏点击事件
        const toggleEdit = () => {
            isEditing.value = true;
        };

        const userinfo = ref({});

        // 获取用户信息
        const getUserInfo = async () => {
            const response = await fetch('/user/getUserInfo');
            userinfo.value = await response.json();
            prePassword.value = userinfo.value.password;
        };

        const selectedFile = ref(null);
        const previewUrl = ref(null);

        // 触发文件选择
        const triggerFileSelect = () => {
            const fileInput = document.getElementById('avatarUpload');
            fileInput.click();
        };

        //处理文件选择
        const handleFileChange = (event) => {
            selectedFile.value = event.target.files[0];
            if (selectedFile.value) {
                previewUrl.value = URL.createObjectURL(selectedFile.value);
            }
            console.log(selectedFile.value);
            console.log(previewUrl.value);
        };

        //修改用户头像
        const uploadHeadPortrait = async () => {
            if (selectedFile.value) {
                console.log('上传头像');
            } else {
                console.log('请选择文件');
            }

            const formData = new FormData();
            formData.append('user_head_portrait', selectedFile.value);

            await fetch('/user/updateUserInfo', {
                method: 'POST',
                body: formData,
            });
            //重新获取用户信息
            getUserInfo();
            //重新获取头像
            previewUrl.value = null;
        };

        const preName = ref('');

        const endEditName = async () => {
            if (preName.value === userinfo.value.name) {
                nameIsReadOnly.value = true;
                return;
            }
            const data = new FormData();
            data.append('name', preName.value);
            await fetch('/user/updateUserInfo', {
                method: 'POST',
                body: data,
            });
            isEditing.value = false;
            //重新获取用户信息
            getUserInfo();
        };

        const prePassword = ref('');
        const newUserName = ref('');
        const newPassword = ref('');
        const againPassword = ref('');

        const endEditUserNameAndPassword = async () => {
            if (newPassword.value != '' && newPassword.value === prePassword.value) {
                alert('新密码与原密码相同');
                isEditing.value = false;
                newPassword.value = '';
                againPassword.value = '';
                return;
            }
            if (newPassword.value != '' && newPassword.value !== againPassword.value) {
                alert('两次输入的密码不一致');
                isEditing.value = false;
                newPassword.value = '';
                againPassword.value = '';
                return;
            }
            const data = new FormData();
            if (newPassword.value != '') {
                data.append('password', newPassword.value);
            }
            if (newUserName.value != '') {
                data.append('username', newUserName.value);
            }
            //没有修改内容
            if (data.length == 0) {
                alert('没有修改内容');
                isEditing.value = false;
                newPassword.value = '';
                againPassword.value = '';
                newUserName.value = '';
                return;
            }
            //账号不为空，而密码为空
            if (newPassword === ' ') {
                data.append('password', userinfo.value.password);
            }
            //账号为空，而密码不为空
            else {
                data.append('username', userinfo.value.username);
            }
            await fetch('/user/updateUserInfo', {
                method: 'POST',
                body: data,
            });
            isEditing.value = false;
            newPassword.value = '';
            againPassword.value = '';
            newUserName.value = '';
            //重新获取用户信息
            getUserInfo();
        };

        // 挂载
        onMounted(() => {
            getUserInfo();
        });

        return {
            navItems,
            activePage,
            handleNavClick,
            userinfo, // Add userinfo here
            uploadHeadPortrait,
            handleFileChange,
            triggerFileSelect,
            previewUrl,
            preName,
            endEditName,
            activePage,
            isEditing,
            toggleEdit,
            endEditUserNameAndPassword,
            newPassword,
            againPassword,
            newUserName,
        };
    },
}).mount('#personal');
const { ref, reactive, onMounted } = Vue;

const personal = Vue.createApp({
    setup() {
        const videoList = ref([]);
        const navItems = ref([
            { page: 'profile', label: '个人资料' },
            { page: 'security', label: '账号安全' },
            { page: 'video', label: '视频管理' },
            { page: 'favorite', label: '你的收藏' },
            { page: 'logout', label: '退出登录' },
        ]);

        const activePage = ref('profile');
        const isEditing = ref(false);
        const isEditMode = ref(false); // 新增的编辑模式状态

        const switchPage = (page) => {
            activePage.value = page;
        };

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

        const toggleEdit = () => {
            isEditing.value = true;
        };

        const toggleEditMode = () => {
            isEditMode.value = !isEditMode.value; // 切换编辑模式
        };

        const deleteVideo = async (videoId) => {
            const userConfirmed = confirm('确定要删除这个视频吗？');
            if (!userConfirmed) {
                return; // 用户取消删除操作
            }
        
            try {
                const response = await fetch('/video/' + videoId, {
                    method: 'DELETE',
                });
        
                if (response.ok) {
                    videoList.value = videoList.value.filter(video => video.video_id !== videoId);
                    console.log('Video deleted successfully');
                } else {
                    console.error('Failed to delete video:', response.statusText);
                }
            } catch (error) {
                console.error('Error deleting video:', error);
            }
        };

        const userinfo = ref({});

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

        const handleFileChange = (event) => {
            selectedFile.value = event.target.files[0];
            if (selectedFile.value) {
                previewUrl.value = URL.createObjectURL(selectedFile.value);
            }
            console.log(selectedFile.value);
            console.log(previewUrl.value);
        };

        const cancelUpload = () => {
            selectedFile.value = null;
            previewUrl.value = null;
        };

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
            getUserInfo();
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
            if (data.length == 0) {
                alert('没有修改内容');
                isEditing.value = false;
                newPassword.value = '';
                againPassword.value = '';
                newUserName.value = '';
                return;
            }
            if (newPassword === ' ') {
                data.append('password', userinfo.value.password);
            } else {
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
            getUserInfo();
        };
        
        const getVideo = async () => {
            const response = await fetch('/user/getVideo?uid=' + userinfo.value.uid);
            videoList.value = await response.json();
            getFavorite();
            console.log(videoList.value);
        }
        
        const update_video_form = ref({
            video_name: '',
            video_info: '',
        });

        const handleUpdateVideo = async (videoId) => {
            const video = videoList.value.find(v => v.video_id === videoId);
            if (!video) {
                console.error('Video not found');
                return;
            }

            // 这里可以添加更新视频信息的逻辑，例如发送请求到服务器
            console.log('Updated Video Name:', update_video_form.value.video_name);
            console.log('Updated Video Info:', update_video_form.value.video_info);
            //发送请求到服务器
            await fetch('/video/' + videoId, {
                method: 'PUT',
                body: JSON.stringify(update_video_form.value),
            });
            // 关闭模态框
            $(`#updatevideo-${videoId}`).modal('hide');
            //清空更新表单
            update_video_form.value.video_name = '';
            update_video_form.value.video_info = '';
            //刷新视频列表
            getVideo();
        };

        //收藏
        const favoriteList = ref([]);

        const getFavorite = async () => {
            const response = await fetch('/user/getFavorite?uid=' + userinfo.value.uid);
            favoriteList.value = await response.json();
            console.log(favoriteList.value);
        }

        onMounted(() => {
            getUserInfo();
            getVideo();
        });

        return {
            navItems,
            activePage,
            handleNavClick,
            userinfo,
            uploadHeadPortrait,
            handleFileChange,
            triggerFileSelect,
            previewUrl,
            preName,
            endEditName,
            activePage,
            isEditing,
            toggleEdit,
            toggleEditMode, // 返回 toggleEditMode
            deleteVideo, // 返回 deleteVideo
            endEditUserNameAndPassword,
            newPassword,
            againPassword,
            newUserName,
            videoList,
            isEditMode, // 返回 isEditMode
            update_video_form,
            handleUpdateVideo,
            favoriteList,
            selectedFile,
            cancelUpload,
        };
    },
}).mount('#personal');
const { ref, onMounted } = Vue;

const tag = Vue.createApp({
    setup() {
        const tags = ref([]);
        const userinfo = ref({});
        const searchQuery = ref('');

        const getUserInfo = async () => {
            try {
                const res = await axios.get('/user/getUserInfo');
                userinfo.value = res.data;
            } catch (error) {
                console.error("Error fetching user info:", error);
            }
        };

        const getTags = async () => {
            try {
                const res = await axios.get('/getTotal?table_name=tag');
                tags.value = res.data;
            } catch (error) {
                console.error("Error fetching tags:", error);
            }
        };

        const redirectToSearch = () => {
            window.location.href = `search.html?search=${searchQuery.value}`;
        };

        onMounted(() => {
            getUserInfo();
            getTags();
        });

        return {
            tags,
            userinfo,
            searchQuery,
            redirectToSearch
        };
    }
}).mount('#tag');

// 头像点击跳转
document.querySelector('.avatar')?.addEventListener('click', function () {
    window.location.href = 'personal.html';
});
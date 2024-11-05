$(document).ready(function() {
    // 导航切换
    $('.nav-item').click(function() {
        // 移除所有active类
        $('.nav-item').removeClass('active');
        $('.content-page').removeClass('active');
        
        // 添加active类到当前项
        $(this).addClass('active');
        
        // 显示对应内容
        const page = $(this).data('page');
        $(`#${page}-page`).addClass('active');

        // 如果点击的是退出登录
        if(page === 'logout') {
            if(confirm('确定要退出登录吗？')) {
                window.location.href = 'login.html';
            } else {
                // 如果取消退出，恢复到个人资料页
                $('.nav-item[data-page="profile"]').click();
            }
        }
    });

    // 当前正在编辑的输入框
    let currentEditing = null;

    // 保存当前编辑的内容
    function saveCurrentEditing() {
        if (currentEditing) {
            const $group = currentEditing;
            const $input = $group.find('.form-control');
            const $button = $group.find('.edit-btn');
            
            // 在这里可以添加保存到服务器的逻辑
            console.log('保存内容:', $input.val());
            
            // 更新UI
            $input.prop('readonly', true);
            $button.removeClass('active');
            $button.html('<i class="fa fa-pencil"></i> 修改');
            
            currentEditing = null;
        }
    }

    // 编辑按钮点击事件
    $('.edit-btn').click(function() {
        const $group = $(this).closest('.edit-group');
        const $input = $group.find('.form-control');
        
        // 如果点击的是其他编辑按钮，先保存当前正在编辑的内容
        if (currentEditing && currentEditing[0] !== $group[0]) {
            saveCurrentEditing();
        }
        
        // 切换编辑状态
        if ($input.prop('readonly')) {
            // 进入编辑状态
            $input.prop('readonly', false);
            $(this).addClass('active');
            $(this).html('<i class="fa fa-check"></i> 保存');
            $input.focus();
            currentEditing = $group;
        } else {
            // 保存编辑
            saveCurrentEditing();
        }
    });

    // 点击页面其他地方时保存
    $(document).click(function(e) {
        if (currentEditing && !$(e.target).closest('.edit-group').length) {
            saveCurrentEditing();
        }
    });

    // 按ESC键取消编辑
    $(document).keyup(function(e) {
        if (e.key === "Escape" && currentEditing) {
            const $input = currentEditing.find('.form-control');
            const $button = currentEditing.find('.edit-btn');
            
            // 恢复原始值
            $input.val($input.prop('defaultValue'));
            $input.prop('readonly', true);
            $button.removeClass('active');
            $button.html('<i class="fa fa-pencil"></i> 修改');
            
            currentEditing = null;
        }
    });
});
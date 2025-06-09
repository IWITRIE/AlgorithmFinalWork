@echo off
setlocal enabledelayedexpansion

REM 配置
set "SOURCE_FILES=berlekamp_massey.cpp gaussian_elimination.cpp"
set "ALGORITHM_NAMES=BM算法 高斯消元法"
set "INPUT_DIR=in"
set "TEMP_OUTPUT_DIR=tmp_compare"
set "ANSWER_DIR=out"
set "COMPILER=g++"
set "CPP_FLAGS=-std=c++11 -O2 -Wno-unused-result"
set "TIMEOUT_SECONDS=30"

REM 统计变量初始化
set "bm_total=0"
set "bm_passed=0"
set "bm_failed=0"
set "bm_timeout=0"
set "bm_total_time=0"

set "gauss_total=0"
set "gauss_passed=0"
set "gauss_failed=0"
set "gauss_timeout=0"
set "gauss_total_time=0"

REM 清理旧文件并创建临时目录
for %%f in (%SOURCE_FILES%) do (
    set "filename=%%f"
    set "executable=!filename:.cpp=!_exec.exe"
    if exist "!executable!" del "!executable!"
)
if exist "%TEMP_OUTPUT_DIR%" rmdir /s /q "%TEMP_OUTPUT_DIR%"
mkdir "%TEMP_OUTPUT_DIR%"

echo =========================================
echo 线性递推算法对比评测系统
echo =========================================
echo 对比算法: BM+NTT算法 vs 高斯消元法
echo.

echo 编译算法实现...
echo -----------------------------------------

REM 编译代码
set "compile_success=1"
set "i=0"
for %%f in (%SOURCE_FILES%) do (
    set /a i+=1
    set "source_file=%%f"
    for %%a in (%ALGORITHM_NAMES%) do (
        if !i! equ 1 set "algorithm_name=%%a" & goto compile
        if !i! equ 2 set "algorithm_name=%%a" & goto compile
    )
    :compile
    set "executable=!source_file:.cpp=!_exec.exe"
    
    echo 编译 !algorithm_name! ...
    %COMPILER% %CPP_FLAGS% "!source_file!" -o "!executable!" >nul 2>&1
    if errorlevel 1 (
        echo 编译失败！
        set "compile_success=0"
    ) else (
        echo 编译成功。
    )
)

if !compile_success! equ 0 (
    echo 编译失败，退出程序
    pause
    exit /b 1
)

echo.

REM 检查目录
if not exist "%INPUT_DIR%" (
    echo 错误：输入目录 '%INPUT_DIR%' 未找到
    pause
    exit /b 1
)

if not exist "%ANSWER_DIR%" (
    echo 错误：答案目录 '%ANSWER_DIR%' 未找到
    pause
    exit /b 1
)

echo 开始对比测试...
echo =========================================

REM 测试每个输入文件
set "test_count=0"
for %%f in ("%INPUT_DIR%\*.in") do (
    set /a test_count+=1
    set "test_case_path=%%f"
    set "test_case_name=%%~nf"
    set "input_file=!test_case_path!"
    set "answer_file=%ANSWER_DIR%\!test_case_name!.out"

    echo 测试用例 !test_count!: !test_case_name!
    echo -----------------------------------------

    REM 检查答案文件
    if not exist "!answer_file!" (
        echo 跳过 ^(未找到答案文件^)
        echo.
        goto next_test
    )

    REM 显示输入信息
    for /f "tokens=1,2" %%a in (!input_file!) do (
        echo 序列长度: %%a, 目标项: %%b
        goto show_info_done
    )
    :show_info_done

    REM 测试两个算法
    set "i=0"
    for %%s in (%SOURCE_FILES%) do (
        set /a i+=1
        set "source_file=%%s"
        for %%a in (%ALGORITHM_NAMES%) do (
            if !i! equ 1 set "algorithm_name=%%a" & goto test_algo
            if !i! equ 2 set "algorithm_name=%%a" & goto test_algo
        )
        :test_algo
        set "executable=!source_file:.cpp=!_exec.exe"
        set "temp_output_file=%TEMP_OUTPUT_DIR%\!test_case_name!_!source_file:.cpp=!.out"

        echo   !algorithm_name!:

        REM 更新统计
        if !i! equ 1 (
            set /a bm_total+=1
        ) else (
            set /a gauss_total+=1
        )

        REM 运行程序并计时
        set "start_time=!time!"
        timeout /t %TIMEOUT_SECONDS% "!executable!" < "!input_file!" > "!temp_output_file!" 2> "!temp_output_file!.err"
        set "program_exit_code=!errorlevel!"
        set "end_time=!time!"

        if !program_exit_code! equ 1 (
            echo     超时 ^(^>%TIMEOUT_SECONDS%s^)
            if !i! equ 1 (
                set /a bm_timeout+=1
            ) else (
                set /a gauss_timeout+=1
            )
            goto next_algo
        )
        if !program_exit_code! neq 0 (
            echo     运行错误 ^(退出码: !program_exit_code!^)
            if !i! equ 1 (
                set /a bm_failed+=1
            ) else (
                set /a gauss_failed+=1
            )
            goto next_algo
        )

        REM 检查答案正确性
        fc /w "!temp_output_file!" "!answer_file!" >nul 2>&1
        if !errorlevel! equ 0 (
            echo     通过
            if !i! equ 1 (
                set /a bm_passed+=1
            ) else (
                set /a gauss_passed+=1
            )
        ) else (
            echo     答案错误
            if !i! equ 1 (
                set /a bm_failed+=1
            ) else (
                set /a gauss_failed+=1
            )
        )

        :next_algo
    )
    echo.
    :next_test
)

echo =========================================
echo 对比测试完成 - 总结报告
echo =========================================
echo.

REM 生成统计报告
echo 算法性能统计:
echo --------------------------------------------------------------------------------
echo BM+NTT算法:
echo   总测试: !bm_total!, 通过: !bm_passed!, 失败: !bm_failed!, 超时: !bm_timeout!
if !bm_total! gtr 0 (
    set /a bm_success_rate=!bm_passed!*100/!bm_total!
    echo   成功率: !bm_success_rate!%%
)

echo.
echo 高斯消元法:
echo   总测试: !gauss_total!, 通过: !gauss_passed!, 失败: !gauss_failed!, 超时: !gauss_timeout!
if !gauss_total! gtr 0 (
    set /a gauss_success_rate=!gauss_passed!*100/!gauss_total!
    echo   成功率: !gauss_success_rate!%%
)

@echo off
setlocal enabledelayedexpansion

REM 配置
set "SOURCE_FILES=berlekamp_massey.cpp matrix_power.cpp poly_multiply.cpp naive.cpp"
set "ALGORITHM_NAMES=NTT优化 矩阵快速幂 多项式乘法 朴素算法"
set "INPUT_DIR=in"
set "TEMP_OUTPUT_DIR=tmp"
set "ANSWER_DIR=out"
set "COMPILER=g++"
set "CPP_FLAGS=-std=c++11 -O2 -Wno-unused-result"

REM 清理旧的可执行文件和创建临时目录
for %%f in (%SOURCE_FILES%) do (
    set "filename=%%f"
    set "executable=!filename:.cpp=!_exec.exe"
    if exist "!executable!" del "!executable!"
)
if exist "%TEMP_OUTPUT_DIR%" rmdir /s /q "%TEMP_OUTPUT_DIR%"
mkdir "%TEMP_OUTPUT_DIR%"

echo =========================================
echo 编译所有算法实现...
echo =========================================

REM 编译所有 C++ 代码
set "i=0"
for %%f in (%SOURCE_FILES%) do (
    set /a i+=1
    set "source_file=%%f"
    for %%a in (%ALGORITHM_NAMES%) do (
        if !i! equ 1 set "algorithm_name=%%a" & goto compile
        if !i! equ 2 set "algorithm_name=%%a" & goto compile
        if !i! equ 3 set "algorithm_name=%%a" & goto compile
        if !i! equ 4 set "algorithm_name=%%a" & goto compile
    )
    :compile
    set "executable=!source_file:.cpp=!_exec.exe"
    
    echo 编译 !source_file! (!algorithm_name!)...
    %COMPILER% %CPP_FLAGS% "!source_file!" -o "!executable!"
    if errorlevel 1 (
        echo 编译失败！
        exit /b 1
    )
    echo 编译成功。
)

echo =========================================
echo 开始测试...
echo =========================================

REM 检查输入目录是否存在
if not exist "%INPUT_DIR%" (
    echo 错误：输入目录 '%INPUT_DIR%' 未找到。
    exit /b 1
)

REM 检查答案目录是否存在
if not exist "%ANSWER_DIR%" (
    echo 错误：答案目录 '%ANSWER_DIR%' 未找到。
    exit /b 1
)

REM 遍历所有输入文件
for %%f in ("%INPUT_DIR%\*.in") do (
    set "test_case_path=%%f"
    set "test_case_name=%%~nf"
    set "input_file=!test_case_path!"
    set "answer_file=%ANSWER_DIR%\!test_case_name!.out"

    echo 测试用例: !test_case_name!
    echo -----------------------------------------

    REM 检查对应的答案文件是否存在
    if not exist "!answer_file!" (
        echo 跳过 ^(未找到答案文件: !answer_file!^)
        echo.
        goto next_test
    )

    REM 测试所有算法
    set "i=0"
    for %%s in (%SOURCE_FILES%) do (
        set /a i+=1
        set "source_file=%%s"
        for %%a in (%ALGORITHM_NAMES%) do (
            if !i! equ 1 set "algorithm_name=%%a" & goto test_algo
            if !i! equ 2 set "algorithm_name=%%a" & goto test_algo
            if !i! equ 3 set "algorithm_name=%%a" & goto test_algo
            if !i! equ 4 set "algorithm_name=%%a" & goto test_algo
        )
        :test_algo
        set "executable=!source_file:.cpp=!_exec.exe"
        set "temp_output_file=%TEMP_OUTPUT_DIR%\!test_case_name!_!source_file:.cpp=!.out"

        echo   !algorithm_name!:

        REM 运行程序并计时
        set "start_time=!time!"
        timeout /t 10 "!executable!" < "!input_file!" > "!temp_output_file!" 2> "!temp_output_file!.err"
        set "program_exit_code=!errorlevel!"
        set "end_time=!time!"

        if !program_exit_code! equ 1 (
            echo 超时 ^(>10s^)
            goto next_algo
        )
        if !program_exit_code! neq 0 (
            echo 运行时错误 ^(退出码: !program_exit_code!^)
            goto next_algo
        )

        REM 比较临时输出文件和答案文件
        fc /w "!temp_output_file!" "!answer_file!" >nul 2>&1
        if !errorlevel! equ 0 (
            echo 通过
        ) else (
            echo 答案错误
        )

        :next_algo
    )
    echo.
    :next_test
)

echo =========================================
echo 测试完成。
echo =========================================

REM 清理临时输出目录和可执行文件
if exist "%TEMP_OUTPUT_DIR%" rmdir /s /q "%TEMP_OUTPUT_DIR%"
for %%f in (%SOURCE_FILES%) do (
    set "filename=%%f"
    set "executable=!filename:.cpp=!_exec.exe"
    if exist "!executable!" del "!executable!"
)

pause
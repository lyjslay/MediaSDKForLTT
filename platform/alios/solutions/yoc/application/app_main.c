/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>

#define TAG "app"

int main(int argc, char *argv[])
{

	while (1) {
		aos_msleep(3000);
	};
}

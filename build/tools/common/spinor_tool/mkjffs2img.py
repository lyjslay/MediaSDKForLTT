#!/usr/bin/python
# -*- coding: utf-8 -*-
import argparse
import logging
from os import path, getcwd
from XmlParser import XmlParser
from tempfile import mkdtemp
import sys
import subprocess

FORMAT = "%(levelname)s: %(message)s"
logging.basicConfig(level=logging.INFO, format=FORMAT)


def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller   """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = path.dirname(path.realpath(__file__))
    return path.join(base_path, relative_path)


def parse_Args():
    parser = argparse.ArgumentParser(description="Create jffs2 image")
    parser.add_argument("xml", help="path to partition xml")
    parser.add_argument("label", help="label of the partition")
    parser.add_argument("input_path", help="input file or folder for packing")
    parser.add_argument("output_path", help="the folder path to install dir inclued fip,rootfs and kernel")
    parser.add_argument("block_size", help="block size", type=int)
    parser.add_argument("jffs2_compression", help="jffs2 compression type")
    parser.add_argument("--mkfs", help="path to mkfs.jffs2", type=str)
    args = parser.parse_args()

    if args.mkfs is None:
        args.mkfs = resource_path("mkfs.jffs2")
    return args


def create_jffs2fs(
    input_dir, output_dir, block_size, compression_type, mkfs, part_size
):
    mkfs_cmd = "%s -e %d -d %s -o %s -U -n -X %s -m size -p %d"%(mkfs, block_size, input_dir, output_dir, compression_type, part_size)
    logging.info("mkfs_cmd:%s", mkfs_cmd)
    try:
        process = subprocess.Popen(
            mkfs_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=getcwd(),
            shell=True,
        )
    except Exception:
        return -1

    ret = process.wait()
    return ret

def main():
    args = parse_Args()
    parser = XmlParser(args.xml)
    parts = parser.parse()
    for p in parts:
        # Since xml parser will parse with abspath and the user input path can
        # be relative path, use file name to check.
        if args.label == p["label"]:
            part = p

            break
    try:
        part_size = part["part_size"]
        label = part["label"]
    except Exception:
        logging.error("label is not found in partition.xml, please check!")
        return -1
    logging.debug("get partition as below:")
    logging.debug(p)

    logging.info("Creating jffs2fs")
    ret = create_jffs2fs(
        args.input_path,
        args.output_path,
        args.block_size,
        args.jffs2_compression,
        args.mkfs,
        part_size
    )
    if ret:
        logging.error("create jffs2fs error, please enable verbose!")
        return -1
    return 0


if __name__ == "__main__":
    main()

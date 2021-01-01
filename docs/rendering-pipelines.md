# Rendering pipelines

## About

This document gathers the ideas to turn the actual rendering process in a
renderig pipeline, which can trace rays in batch.

## Introduction

Rendering libraries and hardware perform in step...

## The pipeline

The pipeline will be comprised of the steps:

* Compute rays from screen coordinates.
* Visit the scene in a batch of rays, every shape is visited by all rays before
  visiting the next. Drop std::optional for an execution mask?
* Ray results are evaluated...
